#pragma once

struct Camera
{
    float4 Position;
    // 16 byte boundary
    float4x4 View;
    float4x4 Projection;
    float4x4 ViewProjection;
    float4x4 InverseView;
    float4x4 InverseProjection;
    float4x4 InverseViewProjection;
    // 16 byte boundary
    float NearPlane;
    float FarPlane;
    float ExposureValue100;
    float FoVH;
    // 16 byte boundary
    float FoVV;
    float FoVHTan;
    float FoVVTan;
    float AspectRatio; // W/H
};


