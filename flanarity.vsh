#version 150

in vec2 in_pos;
in vec2 in_mid;

out vec2 out_pos;
out vec2 out_mid;

void main()
{
	out_pos = in_pos;
    out_mid = in_mid;
    gl_Position = vec4(in_pos, 0.0, 1.0);
}