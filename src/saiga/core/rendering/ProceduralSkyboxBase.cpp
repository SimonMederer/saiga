/**
 * Copyright (c) 2017 Darius Rückert
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#include "ProceduralSkyboxBase.h"

#include "saiga/core/imgui/imgui.h"

#include "internal/noGraphicsAPI.h"
namespace Saiga
{
void ProceduralSkyboxBase::imgui()
{
    ImGui::InputFloat("horizonHeight", &horizonHeight);
    ImGui::InputFloat("distance", &distance);
    ImGui::SliderFloat("sunIntensity", &sunIntensity, 0, 2);
    ImGui::SliderFloat("sunSize", &sunSize, 0, 2);
    ImGui::Direction("sunDir", sunDir);
}

}  // namespace Saiga
