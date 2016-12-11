#include "saiga/rendering/overlay/textDebugOverlay.h"
#include "saiga/opengl/shader/basic_shaders.h"
#include "saiga/geometry/triangle_mesh.h"
#include "saiga/opengl/framebuffer.h"
#include "saiga/opengl/shader/shaderLoader.h"
#include "saiga/text/textureAtlas.h"
#include "saiga/text/textShader.h"
#include "saiga/text/text.h"

TextDebugOverlay::TextDebugOverlay(int w, int h): overlay(1,1),layout(w,h){
}

TextDebugOverlay::~TextDebugOverlay()
{
    for(TDOEntry &entry : entries){
        delete entry.text;
    }
}

void TextDebugOverlay::init(TextureAtlas *textureAtlas)
{
    this->textureAtlas = textureAtlas;
}

void TextDebugOverlay::render()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    overlay.render(&layout.cam);
}

int TextDebugOverlay::createItem(const std::string &name)
{
    int id = entries.size();
    TDOEntry entry;
    entry.valueIndex = name.size();

    entry.text = new Text(textureAtlas,"");
    entry.text->params = textParameters;
    overlay.addText(entry.text);

    entry.text->updateText(name,0);
    aabb bb = entry.text->getAabb();
    bb.growBox(textureAtlas->getMaxCharacter());


    int y = id;

    vec2 relPos(0);
    relPos.x = borderX;
    relPos.y =  1.0f-((y) * (paddingY+textSize) + borderY);

    layout.transform(entry.text,bb,relPos,textSize,Layout::LEFT,Layout::RIGHT);

    entries.push_back(entry);


    entry.text->updateText(std::to_string(123),entry.valueIndex);

    return id;
}

template<>
void TextDebugOverlay::updateEntry<std::string>(int id, std::string v)
{
    entries[id].text->updateText(v,entries[id].valueIndex);
}


