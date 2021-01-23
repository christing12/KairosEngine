#pragma once

#include "Camera.hlsl"

struct GlobalData
{
    uint AnisotropicClampSamplerIdx;
    uint LinearClampSamplerIdx;
    uint PointClampSamplerIdx;
    uint MinSamplerIdx;
    uint MaxSamplerIdx;
};


struct FrameData
{
    float4x4 ViewProjMat;
    float3 EyePosition;
    float padding;
    //Camera CurrentFrameCamera;
    //Camera PreviousFrameCamera;

    //bool IsDenoiserEnabled;
    //bool IsReprojectionHistoryDebugEnabled;
    //bool IsGradientDebugEnabled;
    //bool IsMotionDebugEnabled;
    //bool IsDenoiserAntilagEnabled;
};

#define GlobalDataType GlobalData
#define FrameDataType FrameData
#include "EngineMain.hlsl"

sampler AnisotropicClampSampler() { return Samplers[GlobalDataCB.AnisotropicClampSamplerIdx]; }
sampler LinearClampSampler() { return Samplers[GlobalDataCB.LinearClampSamplerIdx]; }
sampler PointClampSampler() { return Samplers[GlobalDataCB.PointClampSamplerIdx]; }
sampler MinSampler() { return Samplers[GlobalDataCB.MinSamplerIdx]; }
sampler MaxSampler() { return Samplers[GlobalDataCB.MaxSamplerIdx]; }