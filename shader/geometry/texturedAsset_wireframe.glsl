
##GL_VERTEX_SHADER

#version 330
layout(location=0) in vec3 in_position;
layout(location=1) in vec3 in_normal;
layout(location=2) in vec2 in_tex;
layout(location=3) in vec4 in_data;

#include "camera.glsl"
uniform mat4 model;

out vec3 normal;
out vec2 texCoord;

void main() {
//    gl_Position = vec4( in_position, 1 );
    texCoord = in_tex;
    normal = normalize(vec3(view*model * vec4( in_normal, 0 )));
    gl_Position = viewProj *model* vec4(in_position,1);
}





##GL_FRAGMENT_SHADER

#version 330

uniform sampler2D image;

in vec3 normal;
in vec2 texCoord;



layout(location=0) out vec3 out_color;


void main() {
//    vec4 diffColor = texture(image, texCoord);
//    vec3 data = vec3(0,0,0);
//    setGbufferData(vec3(diffColor),normal,data);

    out_color = vec3(0.5f);


}


