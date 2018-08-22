﻿/**
 * Copyright (c) 2017 Darius Rückert
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#include "saiga/cuda/imageProcessing/imageProcessing.h"
#include "saiga/cuda/device_helper.h"
#include "saiga/util/statistics.h"

namespace Saiga {
namespace CUDA {

thrust::device_vector<float>  createGaussianBlurKernel(int radius, float sigma){
    SAIGA_ASSERT(radius <= SAIGA_MAX_CONVOLUTION_RADIUS && radius > 0);
#if 0
    const int ELEMENTS = radius * 2 + 1;
    thrust::host_vector<float> kernel(ELEMENTS);
    float kernelSum = 0.0f;
    float ivar2 = 1.0f/(2.0f*sigma*sigma);
    for (int j=-radius;j<=radius;j++) {
        kernel[j+radius] = (float)expf(-(double)j*j*ivar2);
        kernelSum += kernel[j+radius];
    }
    for (int j=-radius;j<=radius;j++)
        kernel[j+radius] /= kernelSum;
    return thrust::device_vector<float>(kernel);
#else
    std::vector<float> kernel = gaussianBlurKernel<float>(radius,sigma);
    return thrust::device_vector<float>(kernel);
#endif
}


void applyFilterSeparate(ImageView<float> src, ImageView<float> dst, ImageView<float> tmp, ArrayView<float> kernelRow, ArrayView<float> kernelCol){
    convolveRow(src,tmp,kernelRow,kernelRow.size() / 2);
    convolveCol(tmp,dst,kernelCol,kernelCol.size() / 2);
}

void applyFilterSeparateSinglePass(ImageView<float> src, ImageView<float> dst, ArrayView<float> kernel){
    int radius = kernel.size()/2;
    //inner 75 is the fastest for small kernels
    convolveSinglePassSeparateInner75(src,dst,kernel,radius);

#if 0
    return;
    if(radius <= 12 && src.cols < 32 && src.rows < 32)
    {
        convolveSinglePassSeparateInner75(src,dst,kernel,radius);
    }
    else if(radius <= 12)
    {
        convolveSinglePassSeparateInnerShuffle(src,dst,kernel,radius);

    }else
    {
        convolveSinglePassSeparateOuterHalo(src,dst,kernel,radius);
    }
#endif
}


}
}


