struct VertexInput {
    float2 inPosition;
    float3 inColor;
}

struct VertexOutput {
    float3 color;
    float4 pos : SV_Position;
};

struct UniformBuffer {
    float4x4 view;
    float4x4 proj;
};
ConstantBuffer<UniformBuffer> ubo;

struct PushConstants {
    float4x4 model;
};

[[vk::push_constant]]
PushConstants push;

[shader ("vertex")]
VertexOutput vertMain(VertexInput input) {
    VertexOutput output;
    output.pos = mul(ubo.proj, mul(ubo.view, mul(push.model, float4(input.inPosition, 0.0, 1.0))));
    output.color = input.inColor;
    return output;
}

[shader ("fragment")]
float4 fragMain (VertexOutput inVert) : SV_Target
{
    return float4(inVert.color, 1.0);
}
