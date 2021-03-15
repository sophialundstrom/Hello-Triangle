Texture2D tex;

SamplerState samp;

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float2 uv : UV;
    float4 worldPosition: WORLDPOSITION;
};

struct Material
{
    float specularPower;
    float3 padding;
};

struct Light
{
	float4 position;
	float4 color;
    float3 attenuation;
    float range;
};

cbuffer LightMaterialProperties
{
	Material material;
    Light light;
    float4 eyePosition;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    float4 finalColor = float4(0, 0, 0, 1);         

    float3 texColor = tex.Sample(samp, input.uv);       //Sampling texture by given uv's

	float4 P = input.worldPosition;                     //Fragments world position
	float3 N = input.normal;                            //Normal
    float3 L = normalize(light.position - P).xyz;       //Fragment to light vector
    float3 E = normalize(eyePosition - P).xyz;          //Fragment to eye/camera vector
    float3 R = normalize(reflect(-L, N));               //Reflection of -L on Normal
    float D = length(L);                                //Calculate distance between light source and fragment

    //Ambient
    float ambient = 0.1;                                //Environment 

    //Diffuse with attenuation
    if (D > light.range)                                //If too far away
        return float4(texColor * ambient, 1);

    float diffusePower = saturate(dot(N, L));           // 0->1 Depending on the angle between vectors
    float diffuseIntensity = 0.8f;                  
    float diffuse = diffusePower * diffuseIntensity;    //Toning it down
    
    if (diffusePower > 0.0f)                            //If normal is pointing towards camera, calculate attenuation
    {
        diffuse /= (light.attenuation[0] + light.attenuation[1] * D + light.attenuation[2] * D * D);
    }

    //Specular

    //Lightening it up with a constant
    float specIntensity = 1.12;       

    //Similar to diffuse, the angle between the reflection of L and E gives us the values
    float3 specular = pow(saturate(dot(R, E) * specIntensity), material.specularPower) * light.color.xyz;

    //Combine + specular to get a more clear highlight
    finalColor = float4(texColor * (diffuse + ambient) + specular, 1);

    return finalColor;
}