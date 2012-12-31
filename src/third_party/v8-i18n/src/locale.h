// Copyright 2011 the v8-i18n authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef V8_I18N_SRC_LOCALE_H_
#define V8_I18N_SRC_LOCALE_H_

#include "v8/include/v8.h"

namespace v8_i18n {

class Locale {
 public:
  Locale() {}

  // Implementations of window.Locale methods.
  static v8::Handle<v8::Value> JSLocale(const v8::Arguments& args);

  // Infers region id given the locale id, or uses user specified region id.
  // Result is canonicalized.
  // Returns status of ICU operation (maximizing locale or get region call).
  static bool GetBestMatchForRegionID(
      const char* locale_id, v8::Handle<v8::Value> regions, char* result);

 private:
  // Key name for localeID parameter.
  static const char* const kLocaleID;
  // Key name for regionID parameter.
  static const char* const kRegionID;
  // Key name for the icuLocaleID result.
  static const char* const kICULocaleID;
};

}  // namespace v8_i18n

#endif  // V8_I18N_SRC_LOCALE_H_
