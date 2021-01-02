#ifndef _SHADING_MATH_H
#define _SHADING_MATH_

#define PI 3.14159265359f
#define TWO_PI 6.28318530718f
#define PI_OVER_TWO 1.5707963268f
#define PI_SQUARED 9.86960440109f


float2 SphericalSample(float3 v)
{
	float2 uv = float2(atan2(v.z, v.x), asin(-v.y)); // sphereical to cartesian
	uv /= float2(-TWO_PI, PI);
	uv += float2(0.5, 0.5);  // converts from from [-0.5, 0.5] -> [0, 1]
	return uv;
}


#endif