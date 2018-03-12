#version 330

layout (location = 0) in vec2 in_pos;
layout (location = 1) in vec2 in_mid;

out vec2 pos;
out vec2 mid;

uniform vec2 clientpos;
uniform vec2 clientsize;
uniform float invaspect;

void main()
{
	pos = in_pos - clientpos;
	pos.x *= invaspect;
	pos.y = -pos.y;
	mid = in_mid - clientpos;
	mid.x *= invaspect;
	mid.y = -mid.y;

    gl_Position = vec4(pos, 0.0, clientsize.y);
}