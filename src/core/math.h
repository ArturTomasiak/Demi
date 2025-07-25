#pragma once
#include "../include.h"

void math_orthographic_f4x4(float* restrict matrix, float left, float right, float bottom, float top, float near_f, float far_f);
void math_translate_f4x4(float* restrict matrix, float x, float y, float z);
void math_identity_f4x4(float* restrict matrix, float v);
void math_multiply_f4x4(float* restrict dest, const float* restrict mat1, const float* restrict mat2);
void math_scale_f4x4(float* restrict matrix, float sx, float sy, float sz);