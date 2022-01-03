#pragma once

constexpr float kFrameEdge = 3.0f;

constexpr float kBoundingBoxEdge = 12.0f; // BB is cube with center in (0,0,0)

constexpr float kFrameBoundingBoxEdge = kBoundingBoxEdge + 2.0f * kFrameEdge + 1.0f; // BB of frame
