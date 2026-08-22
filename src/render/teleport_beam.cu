#define CUB_DISABLE_BF16_SUPPORT
#define GLM_FORCE_CUDA
#define CUDA_VERSION 12000

namespace std {
	using size_t = ::size_t;
};

using namespace std;

#include "../../libs/glm/glm/glm.hpp"
#include "../../libs/glm/glm/gtc/matrix_transform.hpp"
#include "../../libs/glm/glm/gtx/transform.hpp"
#include "../../libs/glm/glm/gtc/quaternion.hpp"

#include "../utils.cuh"
#include "../HostDeviceInterface.h"
#include "../math.cuh"

using glm::ivec2;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using glm::dot;
using glm::length;
using glm::normalize;
using glm::inverse;
using glm::cross;

__device__ inline float intersectSphere(vec3 ro, vec3 rd, vec3 center, float radius) {
	vec3 oc = ro - center;
	float b = dot(oc, rd);
	float c = dot(oc, oc) - radius * radius;
	float disc = b * b - c;

	if (disc < 0.0f) return -1.0f;

	float s = sqrtf(disc);
	float t0 = -b - s;
	float t1 = -b + s;

	if (t0 > 1e-4f) return t0;
	if (t1 > 1e-4f) return t1;

	return -1.0f;
}

__device__ inline bool validRoundedConeRegion(float t, vec3 ro, vec3 rd, vec3 A, vec3 axis, float a, float b, float ah, float r0, int region) {
	if (t <= 1e-4f) return false;

	vec3 q = ro + t * rd - A;
	float z = dot(q, axis);
	vec3 radial = q - z * axis;
	float rho = sqrtf(fmaxf(dot(radial, radial), 0.0f));
	float k = -b * rho + a * z;

	if (region == 0) return k >= -1e-4f && k <= ah + 1e-4f && (r0 - b * z) >= -1e-4f;
	if (region < 0) return k <= 1e-4f;

	return k >= ah - 1e-4f;
}

__device__ inline void acceptCandidate(float t, vec3 ro, vec3 rd, vec3 A, vec3 axis, float a, float b, float ah, float r0, int region, float& bestT) {
	if (t >= bestT) return;

	if (validRoundedConeRegion(t, ro, rd, A, axis, a, b, ah, r0, region)) {
		bestT = t;
	}
}

__device__ inline void testSphereCap(vec3 ro, vec3 rd, vec3 center, float radius, vec3 A, vec3 axis, float a, float b, float ah, float r0, int region, float& bestT) {
	vec3 oc = ro - center;
	float qb = dot(oc, rd);
	float qc = dot(oc, oc) - radius * radius;
	float disc = qb * qb - qc;

	if (disc < 0.0f) return;

	float s = sqrtf(disc);

	acceptCandidate(-qb - s, ro, rd, A, axis, a, b, ah, r0, region, bestT);
	acceptCandidate(-qb + s, ro, rd, A, axis, a, b, ah, r0, region, bestT);
}

__device__ inline float intersectRoundedCone(vec3 ro, vec3 rd, vec3 A, vec3 B, float r0, float r1) {
	r0 = fmaxf(r0, 0.0001f);
	r1 = fmaxf(r1, 0.0001f);

	vec3 ba = B - A;
	float h2 = dot(ba, ba);

	if (h2 < 1e-10f) {
		return intersectSphere(ro, rd, A, fmaxf(r0, r1));
	}

	float h = sqrtf(h2);
	float dr = r0 - r1;

	if (fabsf(dr) >= h - 1e-6f) {
		return r0 >= r1 ? intersectSphere(ro, rd, A, r0) : intersectSphere(ro, rd, B, r1);
	}

	vec3 axis = ba / h;

	float b = dr / h;
	float a = sqrtf(fmaxf(1.0f - b * b, 0.0f));
	float ah = a * h;

	float bestT = 1e30f;

	vec3 q0 = ro - A;
	float z0 = dot(q0, axis);
	float dz = dot(rd, axis);

	vec3 p0 = q0 - z0 * axis;
	vec3 pd = rd - dz * axis;

	float c0 = r0 - b * z0;
	float cd = -b * dz;

	float Aq = a * a * dot(pd, pd) - cd * cd;
	float Bq = 2.0f * (a * a * dot(p0, pd) - c0 * cd);
	float Cq = a * a * dot(p0, p0) - c0 * c0;

	if (fabsf(Aq) > 1e-8f) {
		float disc = Bq * Bq - 4.0f * Aq * Cq;

		if (disc >= 0.0f) {
			float s = sqrtf(disc);
			float inv2A = 0.5f / Aq;

			acceptCandidate((-Bq - s) * inv2A, ro, rd, A, axis, a, b, ah, r0, 0, bestT);
			acceptCandidate((-Bq + s) * inv2A, ro, rd, A, axis, a, b, ah, r0, 0, bestT);
		}
	}
	else if (fabsf(Bq) > 1e-8f) {
		acceptCandidate(-Cq / Bq, ro, rd, A, axis, a, b, ah, r0, 0, bestT);
	}

	testSphereCap(ro, rd, A, r0, A, axis, a, b, ah, r0, -1, bestT);
	testSphereCap(ro, rd, B, r1, A, axis, a, b, ah, r0, +1, bestT);

	return bestT < 1e29f ? bestT : -1.0f;
}

extern "C" __global__ void kernel_drawTeleportBeam(
	CommonLaunchArgs launchArgs,
	RenderTarget target,
	vec3 start,
	vec3 end,
	float startRadius,
	float endRadius,
	uint32_t color,
	mat4 invViewProj,
	vec3 cameraOrigin,
	int minX,
	int minY,
	int rectWidth,
	int rectHeight
) {
	(void)launchArgs;

	int index = blockIdx.x * blockDim.x + threadIdx.x;
	int numPixels = rectWidth * rectHeight;

	if (index >= numPixels) return;

	int x = minX + index % rectWidth;
	int y = minY + index / rectWidth;

	float ndcX = 2.0f * (float(x) + 0.5f) / float(target.width) - 1.0f;
	float ndcY = 2.0f * (float(y) + 0.5f) / float(target.height) - 1.0f;

	vec4 farH = invViewProj * vec4(ndcX, ndcY, 1.0f, 1.0f);

	if (fabsf(farH.w) < 1e-8f) return;

	vec3 farWorld = vec3(farH.x, farH.y, farH.z) / farH.w;
	vec3 rayDirection = normalize(farWorld - cameraOrigin);

	float t = intersectRoundedCone(cameraOrigin, rayDirection, start, end, startRadius, endRadius);

	if (t < 0.0f) return;

	vec3 hit = cameraOrigin + t * rayDirection;
	vec4 viewHit = target.view * vec4(hit, 1.0f);

	float depth = -viewHit.z;

	if (depth <= 0.0f) return;

	uint64_t packed = (uint64_t(__float_as_uint(depth)) << 32) | uint64_t(color);

	atomicMin(
		(unsigned long long*)&target.framebuffer[x + y * target.width],
		(unsigned long long)packed
	);
}

__device__ inline float teleportClamp01(float v) {
	return fminf(fmaxf(v, 0.0f), 1.0f);
}

__device__ inline vec3 teleportUnpackRGB(uint32_t color) {
	return vec3(
		float((color >> 0) & 0xffu) / 255.0f,
		float((color >> 8) & 0xffu) / 255.0f,
		float((color >> 16) & 0xffu) / 255.0f
	);
}

__device__ inline uint32_t teleportPackRGB(vec3 rgb, uint32_t oldColor) {
	uint32_t r = uint32_t(teleportClamp01(rgb.r) * 255.0f + 0.5f);
	uint32_t g = uint32_t(teleportClamp01(rgb.g) * 255.0f + 0.5f);
	uint32_t b = uint32_t(teleportClamp01(rgb.b) * 255.0f + 0.5f);

	return (oldColor & 0xff000000u) | r | (g << 8) | (b << 16);
}

extern "C" __global__ void kernel_drawTeleportRefinementDisk(
	CommonLaunchArgs args,
	RenderTarget target,
	vec3 center,
	vec3 axisDepth,
	vec3 axisHeight,
	float radius,
	vec4 color,
	mat4 invViewProj,
	vec3 cameraOrigin,
	int minX,
	int minY,
	int rectWidth,
	int rectHeight
) {
	int localID = blockIdx.x * blockDim.x + threadIdx.x;
	int numPixels = rectWidth * rectHeight;
	if (localID >= numPixels) return;

	int localX = localID % rectWidth;
	int localY = localID / rectWidth;

	int x = minX + localX;
	int y = minY + localY;

	if (x < 0 || x >= target.width || y < 0 || y >= target.height) return;

	float ndcX = ((float(x) + 0.5f) / float(target.width)) * 2.0f - 1.0f;
	float ndcY = ((float(y) + 0.5f) / float(target.height)) * 2.0f - 1.0f;

	vec4 farH = invViewProj * vec4(ndcX, ndcY, 1.0f, 1.0f);
	if (fabsf(farH.w) < 1e-6f) return;

	vec3 farPoint = vec3(farH) / farH.w;
	vec3 rayDirection = normalize(farPoint - cameraOrigin);

	vec3 depthAxis = normalize(axisDepth);
	vec3 heightAxis = normalize(axisHeight);
	vec3 normal = normalize(cross(depthAxis, heightAxis));

	float denom = dot(rayDirection, normal);
	if (fabsf(denom) < 1e-6f) return;

	float t = dot(center - cameraOrigin, normal) / denom;
	if (t <= 0.0f) return;

	vec3 hitPoint = cameraOrigin + t * rayDirection;
	vec3 offset = hitPoint - center;

	float diskX = dot(offset, depthAxis);
	float diskY = dot(offset, heightAxis);

	if (diskX * diskX + diskY * diskY > radius * radius) return;

	int pixelID = x + y * target.width;

	uint64_t oldPixel = target.framebuffer[pixelID];
	uint32_t oldColor = uint32_t(oldPixel & 0xffffffffull);

	vec3 dstColor = teleportUnpackRGB(oldColor);
	vec3 srcColor = vec3(color.r, color.g, color.b);
	float alpha = teleportClamp01(color.a);

	vec3 blended = srcColor * alpha + dstColor * (1.0f - alpha);
	uint32_t blendedColor = teleportPackRGB(blended, oldColor);

	target.framebuffer[pixelID] = (oldPixel & 0xffffffff00000000ull) | uint64_t(blendedColor);
}