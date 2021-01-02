// We want to use row major matrices
#pragma pack_matrix(row_major)





struct ModelViewProjection {
	float4x4 modelMat;
	float4x4 viewProjMat;
};

struct CameraData {
	float4x4 viewProjMat;
	float3 cameraPos;
};

struct LightData {
	float3 diffuseColor;
	float3 specularColor;
	float3 position;
	float specular;
};

//ConstantBuffer<ColorMul> ColorMulCB : register(b0);
ConstantBuffer<ModelViewProjection> ModelCB : register(b0);

ConstantBuffer<LightData> LightDataCB : register(b1);
ConstantBuffer<CameraData> CameraCB : register(b2);


