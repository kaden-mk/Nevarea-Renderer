#version 460
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

layout(buffer_reference, std430) readonly buffer VertexBuffer {
    vec2 pos[]; 
};

layout(push_constant) uniform PushConstants {
    uint64_t vertex_buffer_address;
} push;

void main() {
    VertexBuffer vertices = VertexBuffer(push.vertex_buffer_address);
    gl_Position = vec4(vertices.pos[gl_VertexIndex], 0.0, 1.0);
}