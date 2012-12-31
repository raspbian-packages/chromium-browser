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

#ifndef V8_I18N_SRC_LANGUAGE_MATCHER_H_
#define V8_I18N_SRC_LANGUAGE_MATCHER_H_

#include "unicode/uloc.h"
#include "v8/include/v8.h"

namespace v8_i18n {

struct LocaleIDMatch {
  LocaleIDMatch();

  LocaleIDMatch& operator=(const LocaleIDMatch& rhs);

  // Bcp47 locale id - "de-Latn-DE-u-co-phonebk".
  char bcp47_id[ULOC_FULLNAME_CAPACITY];

  // ICU locale id - "de_Latn_DE@collation=phonebk".
  char icu_id[ULOC_FULLNAME_CAPACITY];

  // Score for this locale.
  int score;
};

class LanguageMatcher {
 public:
  // Default locale.
  static const char* const kDefaultLocale;

  // Finds best supported locale for a given a list of locale identifiers.
  // It preserves the extension for the locale id.
  static void GetBestMatchForPriorityList(
      v8::Handle<v8::Array> locale_list, LocaleIDMatch* result);

  // Finds best supported locale for a single locale identifier.
  // It preserves the extension for the locale id.
  static void GetBestMatchForString(
      v8::Handle<v8::String> locale_id, LocaleIDMatch* result);

 private:
  // If langauge subtags match add this amount to the score.
  static const int kLanguageWeight;

  // If script subtags match add this amount to the score.
  static const int kScriptWeight;

  // If region subtags match add this amount to the score.
  static const int kRegionWeight;

  // LocaleID match score has to be over this number to accept the match.
  static const int kThreshold;

  // For breaking ties in priority queue.
  static const int kPositionBonus;

  LanguageMatcher();

  // Compares locale_id to the supported list of locales and returns best
  // match.
  // Returns false if it fails to convert locale id from ICU to BCP47 format.
  static bool CompareToSupportedLocaleIDList(v8::Handle<v8::String> locale_id,
                                             LocaleIDMatch* result);
};

}  // namespace v8_i18n

#endif  // V8_I18N_SRC_LANGUAGE_MATCHER_H_
