#version 330

layout (location = 0) in vec2 in_pos;
layout (location = 1) in vec2 in_mid;

out vec2 pos;
out vec2 mid;

uniform float aspect;

void main()
{
	pos.x = in_pos.x;
	pos.y = -in_pos.y;
    mid.x = in_mid.x;
    mid.y = -in_mid.y;
    gl_Position = vec4(in_pos.x/aspect, -in_pos.y, 0.0, 1.0);
}