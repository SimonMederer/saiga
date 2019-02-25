//
// Created by Peter Eichinger on 2019-01-21.
//

#pragma once

#include "saiga/core/util/easylogging++.h"
#include "saiga/core/util/threadName.h"
#include "saiga/vulkan/Queue.h"

#include "BufferChunkAllocator.h"
#include "BufferMemoryLocation.h"
#include "ChunkAllocation.h"
#include "FitStrategy.h"
#include "ImageMemoryLocation.h"

#include <atomic>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

#include <condition_variable>

namespace Saiga::Vulkan
{
struct VulkanBase;
}

namespace Saiga::Vulkan::Memory
{
class ImageCopyComputeShader;
struct OperationPenalties
{
    float target_small_hole     = 100.0f;
    float source_create_hole    = 200.0f;
    float source_not_last_alloc = 100.0f;
    float source_not_last_chunk = 400.0f;
    float same_chunk            = 500.0f;
};

template <typename T>
class Defragger
{
   protected:
    struct DefragOperation
    {
        T* source;
        vk::DeviceMemory targetMemory;
        FreeListEntry target;
        float weight;


        bool operator<(const DefragOperation& second) const { return this->weight < second.weight; }
    };

    bool valid;
    bool enabled;
    VulkanBase* base;
    vk::Device device;
    BaseChunkAllocator<T>* allocator;
    std::multiset<DefragOperation> defrag_operations;


    std::atomic_bool running, quit;

    std::mutex start_mutex, running_mutex, invalidate_mutex;
    std::condition_variable start_condition;
    std::thread worker;

    void worker_func();

    std::set<vk::DeviceMemory> invalidate_set;

    // Defrag thread functions
    float get_operation_penalty(ConstChunkIterator<T> target_chunk, ConstFreeIterator<T> target_location,
                                ConstChunkIterator<T> source_chunk, ConstAllocationIterator<T> source_location) const;

    void apply_invalidations();

    void run();
    // end defrag thread functions
   public:
    OperationPenalties penalties;
    Defragger(VulkanBase* _base, vk::Device _device, BaseChunkAllocator<T>* _allocator)
        : valid(true),
          enabled(false),
          base(_base),
          device(_device),
          allocator(_allocator),
          defrag_operations(),
          running(false),
          quit(false),
          worker(&Defragger::worker_func, this)
    {
    }


    Defragger(const Defragger& other) = delete;
    Defragger& operator=(const Defragger& other) = delete;

    virtual ~Defragger() { exit(); }

    void exit()
    {
        running = false;
        std::unique_lock<std::mutex> lock(running_mutex);

        {
            std::lock_guard<std::mutex> lock(start_mutex);
            running = false;
            quit    = true;
        }
        start_condition.notify_one();
        worker.join();
    }

    void start();
    void stop();

    void setEnabled(bool enable) { enabled = enable; }

    void invalidate(vk::DeviceMemory memory);
    void invalidate(T* location);

    void find_defrag_ops();
    bool perform_defrag();

   protected:
    virtual bool execute_defrag_operation(const DefragOperation& op) = 0;
};



template <typename T>
void Defragger<T>::start()
{
    if (!valid || !enabled || running)
    {
        return;
    }
    // defrag_operations.clear();
    {
        std::lock_guard<std::mutex> lock(start_mutex);
        running = true;
    }
    start_condition.notify_one();
}

template <typename T>
void Defragger<T>::stop()
{
    if (!valid || !running)
    {
        return;
    }
    running = false;
    {
        std::unique_lock<std::mutex> lock(running_mutex);
    }
}


template <typename T>
void Defragger<T>::worker_func()
{
    Saiga::setThreadName("Defragger");
    if (!valid)
    {
        return;
    }
    while (true)
    {
        std::unique_lock<std::mutex> lock(start_mutex);
        start_condition.wait(lock, [this] { return running || quit; });
        if (quit)
        {
            return;
        }
        std::unique_lock<std::mutex> running_lock(running_mutex);
        if (allocator->chunks.empty())
        {
            continue;
        }

        apply_invalidations();

        run();

        running = false;
    }
}

template <typename T>
void Defragger<T>::run()
{
    bool performed = true;
    while (running && performed)
    {
        find_defrag_ops();
        performed = perform_defrag();
    }
}

template <typename T>
void Defragger<T>::find_defrag_ops()
{
    auto& chunks = allocator->chunks;

    for (auto chunk_iter = chunks.rbegin(); running && chunk_iter != chunks.rend(); ++chunk_iter)
    {
        auto& allocs = chunk_iter->allocations;
        for (auto alloc_iter = allocs.rbegin(); running && alloc_iter != allocs.rend(); ++alloc_iter)
        {
            auto& source = **alloc_iter;

            if (source.is_static())
            {
                continue;
            }
            auto begin = chunks.begin();
            auto end   = (chunk_iter).base();  // Conversion from reverse to normal iterator moves one back
            //
            auto new_place = allocator->strategy->findRange(begin, end, source.size);

            if (new_place.first != end)
            {
                const auto target_iter = new_place.second;
                const auto& target     = *target_iter;

                auto current_chunk = (chunk_iter + 1).base();


                if (current_chunk != new_place.first || target.offset < (**alloc_iter).offset)
                {
                    auto weight =
                        get_operation_penalty(new_place.first, target_iter, current_chunk, (alloc_iter + 1).base());

                    defrag_operations.insert(DefragOperation{&source, new_place.first->chunk->memory, target, weight});
                }
            }
        }
    }
}

template <typename T>
bool Defragger<T>::perform_defrag()
{
    {
        using namespace std::chrono_literals;
    }

    auto op        = defrag_operations.begin();
    bool performed = false;
    while (running && op != defrag_operations.end())
    {
        if (allocator->memory_is_free(op->targetMemory, op->target))
        {
            performed |= execute_defrag_operation(*op);
        }

        op = defrag_operations.erase(op);
    }
    return performed;
}


template <typename T>
float Defragger<T>::get_operation_penalty(ConstChunkIterator<T> target_chunk, ConstFreeIterator<T> target_location,
                                          ConstChunkIterator<T> source_chunk,
                                          ConstAllocationIterator<T> source_location) const
{
    auto& source_ptr = *source_location;
    float weight     = 0;


    if (target_chunk == source_chunk)
    {
        // move inside a chunk should be done after moving to others
        weight += penalties.same_chunk;
    }

    // if the move creates a hole that is smaller than the memory chunk itself -> add weight
    if (target_location->size != source_ptr->size && (target_location->size - source_ptr->size < source_ptr->size))
    {
        weight += penalties.target_small_hole * (1 - (static_cast<float>(source_ptr->size) / target_location->size));
    }

    // If move creates a hole at source -> add weight
    auto next = std::next(source_location);
    if (source_location != source_chunk->allocations.cbegin() && next != source_chunk->allocations.cend())
    {
        auto& next_ptr = *next;
        auto& prev     = *std::prev(source_location);

        if (source_ptr->offset == prev->offset + prev->size &&
            source_ptr->offset + source_ptr->size == next_ptr->offset)
        {
            weight += penalties.source_create_hole;
        }
    }

    // Penalty if allocation is not the last allocation in chunk
    if (next != source_chunk->allocations.cend())
    {
        weight += penalties.source_not_last_alloc;
    }

    if (std::next(source_chunk) != allocator->chunks.cend())
    {
        weight += penalties.source_not_last_chunk;
    }

    return weight;
}

template <typename T>
void Defragger<T>::apply_invalidations()
{
    std::unique_lock<std::mutex> invalidate_lock(invalidate_mutex);
    if (!defrag_operations.empty() && !invalidate_set.empty())
    {
        auto ops_iter = defrag_operations.begin();

        while (ops_iter != defrag_operations.end())
        {
            auto target_mem = ops_iter->targetMemory;
            if (invalidate_set.find(target_mem) != invalidate_set.end())
            {
                ops_iter = defrag_operations.erase(ops_iter);
            }
            else
            {
                ++ops_iter;
            }
        }
        invalidate_set.clear();
    }
}

template <typename T>
void Defragger<T>::invalidate(vk::DeviceMemory memory)
{
    std::unique_lock<std::mutex> invalidate_lock(invalidate_mutex);
    invalidate_set.insert(memory);
}

template <typename T>
void Defragger<T>::invalidate(T* location)
{
    std::unique_lock<std::mutex> invalidate_lock(invalidate_mutex);
    for (auto op_iter = defrag_operations.begin(); op_iter != defrag_operations.end();)
    {
        if (op_iter->source == location)
        {
            op_iter = defrag_operations.erase(op_iter);
        }
        else
        {
            ++op_iter;
        }
    }
}



// using BufferDefragger = Defragger<BufferMemoryLocation>;


// template <>
// void BufferDefragger::execute_defrag_operation(const BufferDefragger::DefragOperation& op);

// template <>
// void Defragger<ImageMemoryLocation>::execute_defrag_operation(
//    const Defragger<ImageMemoryLocation>::DefragOperation& op);

class BufferDefragger : public Defragger<BufferMemoryLocation>
{
   public:
    BufferDefragger(VulkanBase* base, vk::Device device, BaseChunkAllocator<BufferMemoryLocation>* allocator)
        : Defragger(base, device, allocator)
    {
    }

   protected:
    bool execute_defrag_operation(const DefragOperation& op) override;
};

class ImageDefragger : public Defragger<ImageMemoryLocation>
{
   private:
    ImageCopyComputeShader* img_copy_shader;

   public:
    ImageDefragger(VulkanBase* base, vk::Device device, BaseChunkAllocator<ImageMemoryLocation>* allocator,
                   ImageCopyComputeShader* _img_copy_shader);

   protected:
    bool execute_defrag_operation(const DefragOperation& op) override;
};

}  // namespace Saiga::Vulkan::Memory
