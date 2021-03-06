/**
 * Copyright (c) 2017 Darius Rückert
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#pragma once

#include "saiga/config.h"
#include "saiga/core/camera/camera.h"
#include "saiga/core/math/math.h"

#include <memory>
#include <vector>

namespace Saiga
{
class TextShader;
class Text;

class SAIGA_OPENGL_API TextOverlay2D
{
   public:
    mat4 proj;
    std::vector<Text*> texts;

    int width, height;

    TextOverlay2D(const mat4& proj);
    TextOverlay2D(int width = 1, int height = 1);
    void render();
    void render(Camera* camera);

    // text stuff
    void addText(Text* text);
    void removeText(Text* text);

    void loadShader();


   private:
    std::shared_ptr<TextShader> textShader = nullptr;
};

}  // namespace Saiga
