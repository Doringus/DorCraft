/*#version 330 core
in vec2 TexCoord;
out vec4 color;
uniform sampler2D ourTexture1;

void main()
{
	color = texture(ourTexture1, TexCoord);
}
*/
#version 330 core
in vec2 TexCoord;

out vec4 color;

uniform sampler2D ourTexture;

void main()
{
    color = vec4(1.0f, 0.5f, 0.2f, 1.0f);
}