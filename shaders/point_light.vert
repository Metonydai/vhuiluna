#version 450

const vec2 OFFSETS[6] = vec2[]
(
    vec2(-1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, -1.0),
    vec2(1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, 1.0)
);

layout (location = 0) out vec2 fragOffset;

struct PointLight {
    vec4 position; // ignore w
    vec4 color; // w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionMatrix;
    mat4 viewMatrix;
    mat4 inverseViewMatrix;
    vec4 ambientLightColor; // w is intensity
    PointLight pointLight[10];
    int numLights;
} ubo;

layout(push_constant) uniform Push {
    vec4 position;
    vec4 color;
    float radius;
} push;

void main()
{
    fragOffset = OFFSETS[gl_VertexIndex];
    vec3 cameraRightWorld = {ubo.viewMatrix[0][0], ubo.viewMatrix[1][0], ubo.viewMatrix[2][0]};
    vec3 cameraUpWorld = {ubo.viewMatrix[0][1], ubo.viewMatrix[1][1], ubo.viewMatrix[2][1]};

    //vec3 positionWorld = ubo.lightPosition.xyz 
    //    + LIGHT_RADIUS * fragOffset.x * cameraRightWorld
    //    + LIGHT_RADIUS * fragOffset.y * cameraUpWorld;
    
    //gl_Position = ubo.projectionMatrix * ubo.viewMatrix * vec4(positionWorld, 1.0);

    vec4 lightInCameraSpace = ubo.viewMatrix * push.position;
    vec4 positionInCameraSpace = lightInCameraSpace + push.radius * vec4(fragOffset, 0.0, 0.0);
    gl_Position = ubo.projectionMatrix * positionInCameraSpace;
}