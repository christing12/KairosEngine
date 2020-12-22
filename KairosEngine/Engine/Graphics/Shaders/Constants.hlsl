// We want to use row major matrices
#pragma pack_matrix(row_major)





struct ModelViewProjection {
	float4x4 MVP;
};

struct ColorMul {
	float4 multiplier;
};

//ConstantBuffer<ColorMul> ColorMulCB : register(b0);
ConstantBuffer<ModelViewProjection> ModelViewProjectionCB : register(b0);
