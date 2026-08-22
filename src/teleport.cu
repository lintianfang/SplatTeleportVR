#include <cuda_runtime.h>
#include <stdint.h>
#include <limits.h>

__device__ inline float3 sub3(
    const float3& a,
    const float3& b)
{
    return make_float3(
        a.x - b.x,
        a.y - b.y,
        a.z - b.z
    );
}

__device__ inline float3 add3(
    const float3& a,
    const float3& b)
{
    return make_float3(
        a.x + b.x,
        a.y + b.y,
        a.z + b.z
    );
}

__device__ inline float3 mul3(
    const float3& a,
    float s)
{
    return make_float3(
        a.x * s,
        a.y * s,
        a.z * s
    );
}

__device__ inline float dot3(
    const float3& a,
    const float3& b)
{
    return a.x * b.x +
           a.y * b.y +
           a.z * b.z;
}

/*
 * 高 32 位：t 的 float bit
 * 低 32 位：point index
 *
 * t 始终为正数时，IEEE float bit 可以直接比较大小。
 */
__device__ inline unsigned long long packCandidate(
    float t,
    uint32_t pointIndex)
{
    uint32_t tBits = __float_as_uint(t);

    return
        (static_cast<unsigned long long>(tBits) << 32) |
        static_cast<unsigned long long>(pointIndex);
}

extern "C"
__global__ void kernel_findTeleportPoint(
    const float3* positions,
    uint64_t numPoints,

    float3 rayOrigin,
    float3 rayDirection,

    float selectionRadius,
    float minDistance,
    float maxDistance,

    unsigned long long* globalBest)
{
    constexpr int BLOCK_SIZE = 256;

    __shared__ unsigned long long blockBest[BLOCK_SIZE];

    const uint32_t localIndex = threadIdx.x;

    const uint64_t pointIndex =
        static_cast<uint64_t>(blockIdx.x) *
        blockDim.x +
        threadIdx.x;

    unsigned long long candidate = ULLONG_MAX;

    if (pointIndex < numPoints) {

        const float3 p = positions[pointIndex];

        const float3 pointFromOrigin =
            sub3(p, rayOrigin);

        const float t =
            dot3(pointFromOrigin, rayDirection);

        if (t >= minDistance &&
            t <= maxDistance) {

            const float3 closestPoint =
                add3(
                    rayOrigin,
                    mul3(rayDirection, t)
                );

            const float3 offset =
                sub3(p, closestPoint);

            const float distanceSquared =
                dot3(offset, offset);

            const float radiusSquared =
                selectionRadius *
                selectionRadius;

            if (distanceSquared <= radiusSquared) {
                candidate = packCandidate(
                    t,
                    static_cast<uint32_t>(pointIndex)
                );
            }
        }
    }

    blockBest[localIndex] = candidate;

    __syncthreads();

    for (uint32_t stride = BLOCK_SIZE / 2;
         stride > 0;
         stride >>= 1) {

        if (localIndex < stride) {
            blockBest[localIndex] =
                min(
                    blockBest[localIndex],
                    blockBest[localIndex + stride]
                );
        }

        __syncthreads();
    }

    if (localIndex == 0 &&
        blockBest[0] != ULLONG_MAX) {

        atomicMin(
            globalBest,
            blockBest[0]
        );
    }
}