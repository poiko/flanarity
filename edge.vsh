#version 330

layout (location = 0) in vec2 in_pos;
layout (location = 1) in vec2 in_mid;

uniform float aspect;

void main()
{
    gl_Position = vec4(in_mid.x/aspect, -in_mid.y, 0.0, 1.0);
}