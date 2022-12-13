#version 430 core

layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

layout (location = 1) uniform int numMaxInstance;

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

    int grass_type = int(rawInstanceProps[idx].position.w);
    const uint unique_idx = atomicAdd(drawCmds[grass_type].instanceCount, uint(1));
    uint offset = drawCmds[grass_type].baseInstance;
    currValidInstanceProps[unique_idx + offset] = rawInstanceProps[idx];
}