#version 430 core

in vec3 f_viewVertex;
in vec3 f_viewNormal;
in vec3 f_worldVertex;
in vec3 f_worldNormal;
in vec3 f_uv;

layout (location = 0) out vec4 fragColor;

uniform int pixelProcessId;
uniform sampler2D albedoTex;

vec4 withFog(vec4 color) {
    const vec4 FOG_COLOR = vec4(0.0, 0.0, 0.0, 1);
    const float MAX_DIST = 150.0;
    const float MIN_DIST = 120.0;

    float dis = length(f_viewVertex);
    float fogFactor = (MAX_DIST - dis) / (MAX_DIST - MIN_DIST);
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    fogFactor = fogFactor * fogFactor;

    vec4 colorWithFog = mix(FOG_COLOR, color, fogFactor);
    return colorWithFog;
}

vec4 getPlaneColor(float x, float z) {
    float stepSize = 2.0;
    float grad = 0.2;
    vec2 coord = vec2(x, z) / stepSize; //square size in world space
    vec2 frac = fract(coord); //fractional parts of squares
    //interpolation, grad is smoothness of line gradients
    vec2 mult = smoothstep(0.0, grad, frac) - smoothstep(1.0 - grad, 1.0, frac);
    vec4 color0 = vec4(0.9, 0.9, 0.9, 1.0);
    vec4 color1 = vec4(0.5, 0.5, 0.5, 1.0);
    vec4 mColor = mix(color0, color1, mult.x * mult.y);
    return withFog(mColor);
}

void proceduralPlane() {
    vec4 color = vec4(0.0, 0.0, 0.0, 0.0);
    color = color + getPlaneColor(f_worldVertex.x, f_worldVertex.z);
    fragColor = withFog(color);
}

void pureColor() {
    fragColor = vec4(1.0, 0.0, 0.0, 1.0);
}

void texture_mapping() {
    fragColor = texture(albedoTex, vec2(f_uv));
}

void main() {
    if (pixelProcessId == 1) { // texture mapping
        texture_mapping();
    }
    else if (pixelProcessId == 4) {
        proceduralPlane();
    }
    else if (pixelProcessId == 5) {
        pureColor();
    }
    
}