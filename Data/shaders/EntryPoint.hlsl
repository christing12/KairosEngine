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
    Camera CurrCamera;
};

#define GlobalDataType GlobalData
#define FrameDataType FrameData
#include "EngineMain.hlsl"

sampler AnisotropicClampSampler() { return Samplers[GlobalDataCB.AnisotropicClampSamplerIdx]; }
sampler LinearClampSampler() { return Samplers[GlobalDataCB.LinearClampSamplerIdx]; }
sampler PointClampSampler() { return Samplers[GlobalDataCB.PointClampSamplerIdx]; }
sampler MinSampler() { return Samplers[GlobalDataCB.MinSamplerIdx]; }
sampler MaxSampler() { return Samplers[GlobalDataCB.MaxSamplerIdx]; }