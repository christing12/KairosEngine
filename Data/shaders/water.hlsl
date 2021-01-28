struct WaterPassBuffer {
	float4 lightDir;
	float4 waterSurfaceColor;
	float4 waterRefractionColor;
	float4 ssrSettings;
	float4 normalMapScroll;
	float2 normalMapScrollSpeed;
	
};

#define PassDataType WaterPassBuffer

#include "EntryPoint.hlsl"


