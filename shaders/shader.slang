struct VSInput {
    float3 inPosition;
    float3 inColor;
    float2 inTexCoord;
    float3 normal;
};

struct UniformBuffer {
    float4x4 view;
    float4x4 proj;
};

// [[vk::binding(binding number, descriptor set number)]]
[[vk::binding(0, 0)]]
ConstantBuffer<UniformBuffer> ubo;

//===============================================================
// Lighting
//===============================================================
static const uint LIGHT_POINT       = 0;
static const uint LIGHT_DIRECTIONAL = 1;
static const uint LIGHT_AMBIENT     = 2;

struct LightData {
    float4 position;
    float4 direction;
    float intensity;
    uint type;
    float2 padding;
};

static const uint MAX_LIGHTS = 4;

struct LightingData {
    uint lightCount;
    LightData lights[MAX_LIGHTS];
};

[[vk::binding(0, 1)]]
ConstantBuffer<LightingData> lighting;

float calculateLighting(float3 worldPosition, float3 normal) {
    normal = normalize(normal);

    float totalLightIntensity = 0.0;

    for (uint i = 0; i < lighting.lightCount; ++i) {
        LightData light = lighting.lights[i];

        if (light.type == LIGHT_AMBIENT) {
            totalLightIntensity += light.intensity;
        } else if (light.type == LIGHT_DIRECTIONAL) {
            // Direction points in the direction light rays travel.
            // This inverts so L is from the point (to be illuminated) to the light source.
            float3 L = normalize(-light.direction.xyz);

            // Lambertian diffuse lighting
            float diffuse = max(dot(normal, L), 0.0);

            totalLightIntensity += light.intensity * diffuse;
        } else if (light.type == LIGHT_POINT) {
            // Direction from the surface toward the light.
            float3 toLight = light.position.xyz - worldPosition;
            float distanceSquared = dot(toLight, toLight);

            float3 L = normalize(toLight);

            float diffuse = max(dot(normal, L), 0.0);

            // Simple inverse-square distance attenuation.
            float attenuation = 1.0 / max(distanceSquared, 0.01);

            totalLightIntensity += light.intensity * diffuse * attenuation;
        }
    }

    return totalLightIntensity;
}
//===============================================================

struct ObjectTransform {
    float4x4 model;
    uint useTexture;
};
[[vk::push_constant]] ConstantBuffer<ObjectTransform> objectTransform;

struct VSOutput {
    float4 pos : SV_Position;
    float3 color;
    float2 fragTexCoord;
    float3 normal;
    float3 worldPosition;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;
    float4 worldPosition = mul(objectTransform.model, float4(input.inPosition, 1.0));
    output.pos = mul(ubo.proj, mul(ubo.view, worldPosition));
    output.color = input.inColor;
    output.fragTexCoord = input.inTexCoord;
    output.worldPosition = worldPosition.xyz;

    // Transform the normals into world space
    output.normal = normalize(
        mul((float3x3)objectTransform.model, input.normal)
    );

    return output;
}

[[vk::binding(0, 2)]]
Sampler2D texture;

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_TARGET {
    float3 normal = normalize(vertIn.normal);

    float4 baseColor;

    if (objectTransform.useTexture != 0) {
        baseColor = texture.Sample(vertIn.fragTexCoord);
    } else {
        baseColor = float4(vertIn.color, 1.0);
    }

    float illumination = calculateLighting(
        vertIn.worldPosition,
        normal
    );

    return float4(baseColor.rgb * illumination, baseColor.a);
}
