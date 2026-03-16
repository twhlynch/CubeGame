#ifndef RN_UV0
#define RN_UV0 0
#endif

#ifndef RN_COLOR
#define RN_COLOR 0
#endif

#ifndef RN_USE_MULTIVIEW
#define RN_USE_MULTIVIEW 0
#endif

#if RN_UV0
[[vk::binding(2)]] SamplerState linearRepeatSampler;
[[vk::binding(3)]] Texture2D texture0;
#endif

[[vk::binding(0)]] cbuffer vertexUniforms
{
	matrix modelMatrix;

#if RN_USE_MULTIVIEW
	matrix viewProjectionMatrix_multiview[2];
#else
	matrix viewProjectionMatrix;
#endif

	float4 ambientColor;
	float4 diffuseColor;
	
	float3x3 normalMatrix;
};

[[vk::binding(1)]] cbuffer fragmentUniforms
{
	float4 cameraAmbientColor;

	float3 cameraPosition;
};

struct InputVertex
{
	float3 position : POSITION;
	float3 normal : NORMAL;

#if RN_COLOR
	float4 color : COLOR;
#endif

#if RN_UV0
	float2 texCoords : TEXCOORD0;
#endif

#if RN_USE_MULTIVIEW
	uint viewIndex : SV_VIEWID;
#endif
};

struct FragmentVertex
{
	float4 position : SV_POSITION;
	half4 color : TEXCOORD1;

	float3 worldPosition : TEXCOORD2;
	float3 worldNormal : TEXCOORD3;

#if RN_UV0
	half2 texCoords : TEXCOORD0;
#endif
};

FragmentVertex main_vertex(InputVertex vert)
{
	FragmentVertex result;

#if RN_UV0
	// UV
	result.texCoords = vert.texCoords;
#endif

	// position and normal
	float4 position = float4(vert.position, 1.0);

	float4 worldPosition = mul(modelMatrix, position);
	float3 worldNormal = normalize(mul(normalMatrix, vert.normal));

#if RN_USE_MULTIVIEW
	result.position = mul(viewProjectionMatrix_multiview[vert.viewIndex], worldPosition);
#else
	result.position = mul(viewProjectionMatrix, worldPosition);
#endif

	result.worldPosition = worldPosition.xyz;
	result.worldNormal = worldNormal;

	// base color
#if RN_COLOR
	result.color = vert.color * diffuseColor * ambientColor;
#else
	result.color = diffuseColor * ambientColor;
#endif

	return result;
}

half4 main_fragment(FragmentVertex vert) : SV_TARGET
{
	// color
	half4 color = vert.color;

#if RN_UV0
	// texture
	color *= texture0.Sample(linearRepeatSampler, vert.texCoords).rgba;
#endif

	// lighting
	float3 lightDirection = normalize(float3(-0.4, -1.0, -0.6));
	float3 viewDirection = normalize(cameraPosition - vert.worldPosition);
	float3 lightColor = float3(1.0, 1.0, 1.0);

	float3 normal = normalize(vert.worldNormal);

	// diffuse
	float alignment = saturate(dot(normal, -lightDirection));

	float3 diffuse = alignment * lightColor;

	// specular
	float specularStrength = 64.0;
	float3 halfDirection = normalize(viewDirection - lightDirection);
	float specularAngle = saturate(dot(normal, halfDirection));
	float specularity = pow(specularAngle, specularStrength) * alignment;

	float3 specular = specularity * lightColor;

	// ambient
	float ambientStrength = 0.4;

	float3 ambient = cameraAmbientColor.rgb * ambientStrength;

	// add together
	float3 lighting = ambient + diffuse + specular;

	color.rgb *= lighting;

	return color;
}
