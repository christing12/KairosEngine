

#define WIDTH_HEIGHT_EVEN 0     // Both the width and the height of the texture are even.
#define WIDTH_ODD_HEIGHT_EVEN 1 // The texture width is odd and the height is even.
#define WIDTH_EVEN_HEIGHT_ODD 2 // The texture width is even and teh height is odd.
#define WIDTH_HEIGHT_ODD 3      // Both the width and height of the texture are odd.

struct ComputeShaderInput
{
	uint3 GroupID			: SV_GROUPID; // 3d index of thread group
	uint3 GroupThreadID		: SV_GroupThreadID; // 3d index of local thread in thread group
	uint3 DispatchThreadID	: SV_DispatchThreadID; // 3d index of global thread id in dispatch
	uint  GroupIndex		: SV_GroupIndex; // flattened local index of thread in thread group
};

cbuffer GenerateMipsCB : register(b0)
{
	uint SrcMipLevel; 
	uint NumMipLevels; // [1, 4]
	uint SrcDimension;
	bool IsSRGB;
	float2 TexelSize;
}

Texture2D<float> SrcMip : register(t0); // src texture

// max 4 mips written per compute shader?
RWTexture2D<float> OutMip : register(u0);


// 8x8 is good because NV = 32 warpsize, AMD = 64 wavefront (they are multiples of 8)
[numthreads(8, 8, 1)] // 64 threads per thread group 
void main( ComputeShaderInput IN )
{
	float4 Src1 = (float4) 0;

    switch (SrcDimension)
    {
    case WIDTH_HEIGHT_EVEN:
    {
        float2 UV = TexelSize * (IN.DispatchThreadID.xy + 0.5);

        Src1 = SrcMip.SampleLevel(LinearClampSampler, UV, SrcMipLevel);
    }

}