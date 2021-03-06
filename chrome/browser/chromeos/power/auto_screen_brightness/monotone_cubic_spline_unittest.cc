// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/power/auto_screen_brightness/monotone_cubic_spline.h"

#include <algorithm>
#include <random>

#include "testing/gtest/include/gtest/gtest.h"

namespace chromeos {
namespace power {
namespace auto_screen_brightness {

TEST(MonotoneCubicSpline, Interpolation) {
  const std::vector<double> xs = {0,   10,  20,   40,   60,  80,
                                  100, 500, 1000, 2000, 3000};
  const std::vector<double> ys = {0, 5, 10, 15, 20, 25, 30, 40, 60, 80, 1000};

  MonotoneCubicSpline spline(xs, ys);
  EXPECT_EQ(spline.GetControlPointsY().size(), xs.size());

  // Spline's control points get their exact values.
  for (size_t i = 0; i < xs.size(); ++i) {
    EXPECT_DOUBLE_EQ(spline.Interpolate(xs[i]), ys[i]);
  }

  // Data points falling out of the range get boundary values.
  EXPECT_DOUBLE_EQ(spline.Interpolate(-0.1), ys[0]);
  EXPECT_DOUBLE_EQ(spline.Interpolate(4000), ys.back());

  // Check interpolation results on non-control points. Results are compared
  // with java implementation of Spline for Android.
  const std::vector<double> ts = {2.2, 4.8, 12.3, 46.4, 70.1, 90.5, 95.8};
  const std::vector<double> expected = {
      1.1,    2.3999999999999995, 6.200916250000001, 16.599999999999998,
      22.525, 28.08849264366124,  29.413985177197368};
  for (size_t i = 0; i < ts.size(); ++i) {
    EXPECT_DOUBLE_EQ(spline.Interpolate(ts[i]), expected[i]);
  }
}

TEST(MonotoneCubicSpline, Monotonicity) {
  const unsigned seed = 1;
  std::default_random_engine generator(seed);
  std::uniform_real_distribution<double> distribution(0.0, 200);

  std::vector<double> xs;
  std::vector<double> ys;
  for (size_t i = 0; i < 10; ++i) {
    xs.push_back(distribution(generator));
    ys.push_back(distribution(generator));
  }

  // Sort xs and ensure they are strictly increasing.
  std::sort(xs.begin(), xs.end());
  for (size_t i = 1; i < xs.size(); ++i) {
    if (xs[i] <= xs[i - 1]) {
      xs[i] = xs[i - 1] + 1;
    }
  }

  std::sort(ys.begin(), ys.end());

  MonotoneCubicSpline spline(xs, ys);

  std::vector<double> test_points;
  for (size_t i = 0; i < 1000; ++i) {
    test_points.push_back(distribution(generator));
  }
  std::sort(test_points.begin(), test_points.end());

  for (size_t i = 1; i < test_points.size(); ++i) {
    EXPECT_LE(spline.Interpolate(test_points[i - 1]),
              spline.Interpolate(test_points[i]));
  }
}

}  // namespace auto_screen_brightness
}  // namespace power
}  // namespace chromeos
