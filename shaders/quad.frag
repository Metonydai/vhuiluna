#version 450

layout (binding = 1) uniform sampler2D samplerColor;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;

struct PointLight {
    vec4 position; // ignore w
    vec4 color; // w is intensity
};

layout (binding = 0) uniform GlobalUbo {
    mat4 projectionMatrix;
    mat4 viewMatrix;
    mat4 inverseViewMatrix;
    mat4 modelMatrix;
    mat4 depthViewProjection;
    vec4 ambientLightColor; // w is intensity
    PointLight pointLight[10];
    int numLights;
    float zNear;
    float zFar;
} ubo;

float LinearizeDepth(float depth)
{
  float n = ubo.zNear;
  float f = ubo.zFar;
  float z = depth;
  return (2.0 * n) / (f + n - z * (f - n));	
}

void main() 
{
	float depth = texture(samplerColor, inUV).r;
	outFragColor = vec4(vec3(LinearizeDepth(depth)), 1.0);

}