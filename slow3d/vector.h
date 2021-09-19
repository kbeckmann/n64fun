#pragma once

#include "types.h"

// in and out can point to the same memory
void vector_transform(const Matrix4f *m, const Vector4f *in, Vector4f *out);
