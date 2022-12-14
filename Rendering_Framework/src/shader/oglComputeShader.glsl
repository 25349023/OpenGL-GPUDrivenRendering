#version 430 core

layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

layout (location = 1) uniform int numMaxInstance;
layout (location = 2) uniform mat4 viewProjMat;

struct DrawCommand {
    uint count;
    uint instanceCount;
    uint firstIndex;
    uint baseVertex;
    uint baseInstance;
};

struct InstanceProperties {
    // last element: type of grass (0 or 1 or 2)
    vec4 position;
};

/*the buffer for storing “whole” instance position and other necessary information*/
layout (std430, binding = 1) buffer RawInstanceData {
    InstanceProperties rawInstanceProps[];
};

/*the buffer for storing “visible” instance position*/
layout (std430, binding = 2) buffer CurrValidInstanceData {
    InstanceProperties currValidInstanceProps[];
};

layout (std430, binding = 3) buffer DrawCommandData {
    DrawCommand drawCmds[];
};

void main() {
    const uint idx = gl_GlobalInvocationID.x;
    if (idx >= numMaxInstance) {
        return;
    }

    vec4 clipCoord = viewProjMat * vec4(rawInstanceProps[idx].position.xyz, 1.0);
    clipCoord /= clipCoord.w;

    bool insideFrustum = (-1.0 <= clipCoord.x) && (clipCoord.x <= 1.0)
            && (-1.0 <= clipCoord.y) && (clipCoord.y <= 1.0)
            && (-1.0 <= clipCoord.z) && (clipCoord.z <= 1.0);

    if (insideFrustum) {
        int grass_type = int(rawInstanceProps[idx].position.w);
        const uint unique_idx = atomicAdd(drawCmds[grass_type].instanceCount, uint(1));
        uint offset = drawCmds[grass_type].baseInstance;
        currValidInstanceProps[unique_idx + offset] = rawInstanceProps[idx];
    }
}