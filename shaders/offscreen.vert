#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

layout (binding = 0) uniform UBO 
{
	mat4 depthVP;
} ubo;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

layout(push_constant) uniform Push {
    mat4 model;
    mat4 normal;
} push;
 
void main()
{
	gl_Position =  ubo.depthVP * push.model * vec4(inPos, 1.0);
}