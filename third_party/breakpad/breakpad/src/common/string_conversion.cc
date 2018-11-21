// Copyright (c) 2006, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <string.h>

#include <unicode/ustring.h>
#include "common/scoped_ptr.h"
#include "common/string_conversion.h"
#include "common/using_std_string.h"

namespace google_breakpad {

using std::vector;

int UTF8ToUTF16Char(const char *in, int in_length, UChar out[2]) {
  int dest_length = 0;
  UErrorCode result = U_ZERO_ERROR;
  u_strFromUTF8(out, 2, &dest_length, in, in_length, &result);

  if (U_FAILURE(result)) {
    out[0] = out[1] = 0;
  }

  return dest_length;
}

void UTF32ToUTF16Char(wchar_t in, UChar out[2]) {
  int32_t dest_length = 0;
  UErrorCode result = U_ZERO_ERROR;
  u_strFromWCS(out, 2, &dest_length, &in, 1, &result);

  if (U_FAILURE(result)) {
    out[0] = out[1] = 0;
  }
}

static inline uint16_t Swap(uint16_t value) {
  return (value >> 8) | static_cast<uint16_t>(value << 8);
}

}  // namespace google_breakpad
