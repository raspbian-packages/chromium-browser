//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Color.h : Defines the Color type used throughout the ANGLE libraries

#ifndef COMMON_COLOR_H_
#define COMMON_COLOR_H_

#include <cstdint>

namespace angle
{

template <typename T>
struct Color
{
    Color();
    Color(T r, T g, T b, T a);

    const T *data() const { return &red; }
    T *ptr() { return &red; }

    static Color fromData(const T *data) { return Color(data[0], data[1], data[2], data[3]); }
    void writeData(T *data) const
    {
        data[0] = red;
        data[1] = green;
        data[2] = blue;
        data[3] = alpha;
    }

    T red;
    T green;
    T blue;
    T alpha;
};

template <typename T>
bool operator==(const Color<T> &a, const Color<T> &b);

template <typename T>
bool operator!=(const Color<T> &a, const Color<T> &b);

typedef Color<float> ColorF;
typedef Color<int> ColorI;
typedef Color<unsigned int> ColorUI;

struct DepthStencil
{
    DepthStencil() : depth(0), stencil(0) {}

    // Double is needed to represent the 32-bit integer range of GL_DEPTH_COMPONENT32.
    double depth;
    uint32_t stencil;
};
}  // namespace angle

// TODO: Move this fully into the angle namespace
namespace gl
{

template <typename T>
using Color   = angle::Color<T>;
using ColorF  = angle::ColorF;
using ColorI  = angle::ColorI;
using ColorUI = angle::ColorUI;

}  // namespace gl

#include "Color.inl"

#endif  // COMMON_COLOR_H_
