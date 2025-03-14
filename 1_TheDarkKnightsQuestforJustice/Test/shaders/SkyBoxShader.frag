#version 450#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D skybox;layout(binding = 2) uniform sampler2D stars;

void main() {	float yaw = -(atan(fragTexCoord.x, fragTexCoord.z)/6.2831853+0.5);	float pitch = -(atan(fragTexCoord.y, sqrt(fragTexCoord.x*fragTexCoord.x+fragTexCoord.z*fragTexCoord.z))/3.14159265+0.5);	outColor = texture(skybox, vec2(yaw, pitch))*0.9+texture(stars, vec2(yaw, pitch))*0.1;
}