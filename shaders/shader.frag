#version 450

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inUV;

layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 0) uniform sampler2D textSampler;

void main() {
    // vec4 samp = ;
    // outColor = vec4(inColor, 1.0) * texture(textSampler, inUV);
    outColor = texture(textSampler, inUV);
}
