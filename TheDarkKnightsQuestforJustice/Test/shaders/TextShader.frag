#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D texSampler;

// adjust the color (red)
const vec4 FGcolor = vec4(1.0f, 0.0f, 0.0f, 1.0f); // we set it in red, const vec4 GREEN = vec4(0.0, 1.0, 0.0, 1.0);  const vec4 BLUE = vec4(0.0, 0.0, 1.0, 1.0);
const vec4 BGcolor = vec4(0.0f, 0.0f, 0.0f, 0.0f); // transparent background
const vec4 SHcolor = vec4(0.0f, 0.0f, 0.0f, 0.0f); // Transparent shadow

void main() {
    vec4 Tx = texture(texSampler, fragTexCoord);
    // we only use red to calculate the color
    outColor = Tx.r * FGcolor + Tx.g * BGcolor + Tx.b * SHcolor;
}
