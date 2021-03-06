// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_INVALIDATION_IMPL_INVALIDATION_PREFS_H_
#define COMPONENTS_INVALIDATION_IMPL_INVALIDATION_PREFS_H_

namespace invalidation {
namespace prefs {

extern const char kInvalidatorClientId[];

extern const char kInvalidatorInvalidationState[];

extern const char kInvalidatorSavedInvalidations[];

extern const char kInvalidationServiceUseGCMChannel[];

extern const char kFCMInvalidationClientIDCache[];

}  // namespace prefs
}  // namespace invalidation

#endif  // COMPONENTS_INVALIDATION_IMPL_INVALIDATION_PREFS_H_
