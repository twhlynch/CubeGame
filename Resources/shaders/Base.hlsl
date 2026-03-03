#ifndef RN_UV0
#define RN_UV0 0
#endif

#ifndef RN_COLOR
#define RN_COLOR 0
#endif

#if RN_UV0
[[vk::binding(3)]] SamplerState linearRepeatSampler : register(s0);
[[vk::binding(4)]] Texture2D texture0 : register(t0);
#endif

[[vk::binding(1)]] cbuffer vertexUniforms : register(b0)
{
	matrix modelMatrix;
	matrix modelViewProjectionMatrix;

	float4 ambientColor;
	float4 diffuseColor;
};

[[vk::binding(2)]] cbuffer fragmentUniforms : register(b1)
{
	float4 cameraAmbientColor;
};

struct InputVertex
{
	[[vk::location(0)]] float3 position : POSITION;
	[[vk::location(1)]] float3 normal : NORMAL;

#if RN_COLOR
	[[vk::location(3)]] float4 color : COLOR;
#endif

#if RN_UV0
	[[vk::location(5)]] float2 texCoords : TEXCOORD0;
#endif
};

struct FragmentVertex
{
	float4 position : SV_POSITION;
	half4 color : TEXCOORD1;

#if RN_UV0
	half2 texCoords : TEXCOORD0;
#endif
};

FragmentVertex main_vertex(InputVertex vert)
{
	FragmentVertex result;

#if RN_UV0
	result.texCoords = vert.texCoords;
#endif

	float4 position = float4(vert.position, 1.0);
	float4 normal = float4(vert.normal, 0.0);

	result.position = mul(modelViewProjectionMatrix, position);

#if RN_COLOR
	result.color = vert.color * diffuseColor * ambientColor;
#else
	result.color = diffuseColor * ambientColor;
#endif

	return result;
}

half4 main_fragment(FragmentVertex vert) : SV_TARGET
{
	half4 color = vert.color;

#if RN_UV0
	color *= texture0.Sample(linearRepeatSampler, vert.texCoords).rgba;
#endif

	color.rgb *= cameraAmbientColor.rgb;
	return color;
}
