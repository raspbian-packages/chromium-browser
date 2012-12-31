// Copyright 2012 the v8-i18n authors.
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

#include "src/locale-list.h"

#include <string.h>

#include "unicode/brkiter.h"
#include "unicode/coll.h"
#include "unicode/datefmt.h"
#include "unicode/numfmt.h"
#include "unicode/uloc.h"
#include "unicode/uversion.h"

namespace v8_i18n {

v8::Handle<v8::Value> JSCanonicalizeLanguageTag(const v8::Arguments& args) {
  v8::HandleScope handle_scope;

  // Expect locale id which is a string.
  if (args.Length() != 1 || !args[0]->IsString()) {
    return v8::ThrowException(v8::Exception::SyntaxError(
        v8::String::New("Locale identifier, as a string, is required.")));
  }

  UErrorCode error = U_ZERO_ERROR;

  char icu_result[ULOC_FULLNAME_CAPACITY];
  int icu_length = 0;

  // Return value which denotes invalid language tag.
  const char* const kInvalidTag = "invalid-tag";

  v8::String::AsciiValue localeID(args[0]->ToString());

  uloc_forLanguageTag(*localeID, icu_result, ULOC_FULLNAME_CAPACITY,
                      &icu_length, &error);
  if (U_FAILURE(error) || icu_length == 0) {
    return v8::String::New(kInvalidTag);
  }

  char result[ULOC_FULLNAME_CAPACITY];

  // Force strict BCP47 rules.
  uloc_toLanguageTag(icu_result, result, ULOC_FULLNAME_CAPACITY, TRUE, &error);

  if (U_FAILURE(error)) {
    return v8::String::New(kInvalidTag);
  }

  return v8::String::New(result);
}

v8::Handle<v8::Value> JSAvailableLocalesOf(const v8::Arguments& args) {
  v8::HandleScope handle_scope;

  // Expect service name which is a string.
  if (args.Length() != 1 || !args[0]->IsString()) {
    return v8::ThrowException(v8::Exception::SyntaxError(
        v8::String::New("Service identifier, as a string, is required.")));
  }

  const icu::Locale* available_locales = NULL;

  int32_t count = 0;
  v8::String::AsciiValue service(args[0]->ToString());
  if (strcmp(*service, "collator") == 0) {
    available_locales = icu::Collator::getAvailableLocales(count);
  } else if (strcmp(*service, "numberformat") == 0) {
    available_locales = icu::NumberFormat::getAvailableLocales(count);
  } else if (strcmp(*service, "dateformat") == 0) {
    available_locales = icu::DateFormat::getAvailableLocales(count);
  } else if (strcmp(*service, "breakiterator") == 0) {
    available_locales = icu::BreakIterator::getAvailableLocales(count);
  }

  UErrorCode error = U_ZERO_ERROR;
  char result[ULOC_FULLNAME_CAPACITY];
  v8::Handle<v8::Object> locales = v8::Object::New();

  for (int32_t i = 0; i < count; ++i) {
    const char* icu_name = available_locales[i].getName();

    error = U_ZERO_ERROR;
    // No need to force strict BCP47 rules.
    uloc_toLanguageTag(icu_name, result, ULOC_FULLNAME_CAPACITY, FALSE, &error);
    if (U_FAILURE(error)) {
      // This shouldn't happen, but lets not break the user.
      continue;
    }

    // Index is just a dummy value for the property value.
    locales->Set(v8::String::New(result, strlen(result)), v8::Integer::New(i));
  }

  // Append 'und' to the list of supported locales.
  locales->Set(v8::String::New("und"), v8::Integer::New(count));

  return handle_scope.Close(locales);
}

}  // namespace v8_i18n
