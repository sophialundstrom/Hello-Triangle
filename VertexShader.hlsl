struct VertexInput 
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 uv : UV;
};

struct VertexOutput
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float2 uv : UV;
	float4 worldPosition: WORLDPOSITION;
};

cbuffer CBuf
{
	float4x4 worldViewPerspective;
	float4x4 world;
};

VertexOutput main(VertexInput input)
{
	VertexOutput output;

	output.pos = mul(float4(input.pos, 1), worldViewPerspective);

	output.normal = normalize(mul(input.normal, world));

	output.uv = input.uv;

	//will output the world position of pixel (interpolation)
	output.worldPosition = mul(input.pos, world);
	
	return output;
}