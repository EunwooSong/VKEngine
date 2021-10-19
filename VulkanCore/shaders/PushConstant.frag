#version 450

layout (location = 0) in vec4 color;
layout (location = 0) out vec4 outColor;

layout(push_constant) uniform colorBlock {
    int constColor;
    float mixerValue;
} pushConstantsColorBlock;

vec4 red   = vec4(1.0, 0.0, 0.0, 1.0);
vec4 green = vec4(0.0, 1.0, 0.0, 1.0);
vec4 blue  = vec4(0.0, 0.0, 1.0, 1.0);

void main() 
{
    if (pushConstantsColorBlock.constColor == 1)
       outColor = red;
    else if (pushConstantsColorBlock.constColor == 2)
       outColor = green;
    else if (pushConstantsColorBlock.constColor == 3)
       outColor = blue;
	else {
		outColor = color;
        outColor.a = pushConstantsColorBlock.mixerValue;
    }
}