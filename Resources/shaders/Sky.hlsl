cbuffer vertexUniforms
{
#if RN_USE_MULTIVIEW
	matrix modelViewMatrix_multiview[6];
	matrix projectionMatrix_multiview[6];
#else
	matrix modelViewMatrix;
	matrix projectionMatrix;
#endif
};

cbuffer fragmentUniforms
{
};

struct InputVertex
{
	[[vk::location(0)]] float3 position : POSITION;

#if RN_USE_MULTIVIEW
	uint viewIndex : SV_VIEWID;
#endif
};

struct FragmentVertex
{
	float4 position : SV_POSITION;
};

FragmentVertex sky_vertex(InputVertex vert)
{
	FragmentVertex result;

#if RN_USE_MULTIVIEW
	float3 rotatedPosition = mul(modelViewMatrix_multiview[vert.viewIndex], float4(vert.position, 0.0)).xyz;
	result.position = mul(projectionMatrix_multiview[vert.viewIndex], float4(rotatedPosition, 0.0)).xyzw;
	result.position.z = 0.0;
#else
	float3 rotatedPosition = mul(modelViewMatrix, float4(vert.position, 0.0)).xyz;
	result.position = mul(projectionMatrix, float4(rotatedPosition, 0.0)).xyzw;
	result.position.z = 0.0;
#endif

	return result;
}

float4 sky_fragment(FragmentVertex vert) : SV_TARGET
{
	// no color, allow seeing passthrough
	return float4(0.0.xxxx);
}
