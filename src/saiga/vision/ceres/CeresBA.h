﻿/**
 * Copyright (c) 2017 Darius Rückert
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#pragma once

#include "saiga/vision/ba/BABase.h"

namespace Saiga
{
class SAIGA_GLOBAL CeresBA : public BABase
{
   public:
    CeresBA() : BABase("Ceres BA") {}
    virtual void solve(Scene& scene, const BAOptions& options) override;
};

}  // namespace Saiga
