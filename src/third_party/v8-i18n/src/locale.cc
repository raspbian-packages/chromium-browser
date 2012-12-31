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

#include "src/locale.h"

#include "src/language-matcher.h"
#include "src/utils.h"
#include "unicode/locid.h"
#include "unicode/uloc.h"

namespace v8_i18n {

const char* const Locale::kLocaleID = "localeID";
const char* const Locale::kRegionID = "regionID";
const char* const Locale::kICULocaleID = "icuLocaleID";

v8::Handle<v8::Value> Locale::JSLocale(const v8::Arguments& args) {
  v8::HandleScope handle_scope;

  if (args.Length() != 1 || !args[0]->IsObject()) {
    return v8::Undefined();
  }

  v8::Local<v8::Object> settings = args[0]->ToObject();

  // Get best match for locale.
  v8::TryCatch try_catch;
  v8::Handle<v8::Value> locale_id = settings->Get(v8::String::New(kLocaleID));
  if (try_catch.HasCaught()) {
    return v8::Undefined();
  }

  LocaleIDMatch result;
  if (locale_id->IsArray()) {
    LanguageMatcher::GetBestMatchForPriorityList(
        v8::Handle<v8::Array>::Cast(locale_id), &result);
  } else if (locale_id->IsString()) {
    LanguageMatcher::GetBestMatchForString(locale_id->ToString(), &result);
  } else {
    LanguageMatcher::GetBestMatchForString(v8::String::New(""), &result);
  }

  // Get best match for region.
  char region_id[ULOC_COUNTRY_CAPACITY];
  Utils::StrNCopy(region_id, ULOC_COUNTRY_CAPACITY, "");

  v8::Handle<v8::Value> region = settings->Get(v8::String::New(kRegionID));
  if (try_catch.HasCaught()) {
    return v8::Undefined();
  }

  if (!GetBestMatchForRegionID(result.icu_id, region, region_id)) {
    // Set region id to empty string because region couldn't be inferred.
    Utils::StrNCopy(region_id, ULOC_COUNTRY_CAPACITY, "");
  }

  // Build JavaScript object that contains bcp and icu locale ID and region ID.
  v8::Handle<v8::Object> locale = v8::Object::New();
  locale->Set(v8::String::New(kLocaleID), v8::String::New(result.bcp47_id));
  locale->Set(v8::String::New(kICULocaleID), v8::String::New(result.icu_id));
  locale->Set(v8::String::New(kRegionID), v8::String::New(region_id));

  return handle_scope.Close(locale);
}

bool Locale::GetBestMatchForRegionID(
    const char* locale_id, v8::Handle<v8::Value> region_id, char* result) {
  if (region_id->IsString() && region_id->ToString()->Length() != 0) {
    icu::Locale user_locale(
        icu::Locale("und", *v8::String::Utf8Value(region_id->ToString())));
    Utils::StrNCopy(
        result, ULOC_COUNTRY_CAPACITY, user_locale.getCountry());
    return true;
  }
  // Maximize locale_id to infer the region (e.g. expand "de" to "de-Latn-DE"
  // and grab "DE" from the result).
  UErrorCode status = U_ZERO_ERROR;
  char maximized_locale[ULOC_FULLNAME_CAPACITY];
  uloc_addLikelySubtags(
      locale_id, maximized_locale, ULOC_FULLNAME_CAPACITY, &status);
  uloc_getCountry(maximized_locale, result, ULOC_COUNTRY_CAPACITY, &status);

  return !U_FAILURE(status);
}

}  // namespace v8_i18n
