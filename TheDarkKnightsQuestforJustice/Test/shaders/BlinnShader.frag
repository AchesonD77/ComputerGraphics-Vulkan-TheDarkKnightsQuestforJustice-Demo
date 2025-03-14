#version 450
#extension GL_ARB_separate_shader_objects : enable

// input variables, passed in from Vertex Shader
layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;

// output colors
layout(location = 0) out vec4 outColor;

// Global Uniforms (Set 0, Binding 0)
layout(set = 0, binding = 0) uniform GlobalUniformBufferObject {
    vec3 lightDir;
    vec4 lightColor;
    vec3 eyePos;
    vec3 spotlightPos;  // flashlight position
    vec3 spotlightDir;  // flashlight direction
    float spotlightCutoff; // flashlight angle
} gubo;

// texture (Set 1, Binding 1)
layout(set = 1, binding = 1) uniform sampler2D tex;

void main() {
    vec3 Norm = normalize(fragNorm);
    vec3 EyeDir = normalize(gubo.eyePos - fragPos);
    
    vec3 lightDir = gubo.lightDir;
    vec3 lightColor = gubo.lightColor.rgb;

    // computing illumination model（Blinn + Lambert）
    vec3 Diffuse = texture(tex, fragUV).rgb * 0.95f * max(dot(Norm, lightDir), 0.0);
    vec3 Specular = vec3(pow(max(dot(Norm, normalize(lightDir + EyeDir)), 0.0), 200.0f));
    vec3 Ambient = texture(tex, fragUV).rgb * 0.05f;
    
    vec3 col = (Diffuse + Specular) * lightColor + Ambient;

    // setting for flashlight
    vec3 lightVec = normalize(gubo.spotlightPos - fragPos); // direction
    float theta = dot(lightVec, normalize(-gubo.spotlightDir)); // angle
    if (theta > gubo.spotlightCutoff) {
        float gaussianFalloff = exp(-pow((1.0 - theta) * 15.0, 2.0)); // using a Gaussian distribution
        float intensity = gaussianFalloff * 3.5; // adjust the level of lights
        col *= texture(tex, fragUV).rgb * intensity ; // if we want some other colos we can use: * vec3(1.5, 1.2, 1.0) (blue light)
    } else {
        col *= 0.1;
    }
    outColor = vec4(col, 1.0f);
}
