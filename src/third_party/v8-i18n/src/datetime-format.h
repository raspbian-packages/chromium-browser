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

#ifndef V8_I18N_SRC_DATETIME_FORMAT_H_
#define V8_I18N_SRC_DATETIME_FORMAT_H_

#include "unicode/uversion.h"
#include "v8/include/v8.h"

namespace U_ICU_NAMESPACE {
class SimpleDateFormat;
}

namespace v8_i18n {

class DateTimeFormat {
 public:
  static v8::Handle<v8::Value> JSDateTimeFormat(const v8::Arguments& args);

  // Helper methods for various bindings.

  // Unpacks date format object from corresponding JavaScript object.
  static icu::SimpleDateFormat* UnpackDateTimeFormat(
      v8::Handle<v8::Object> obj);

  // Release memory we allocated for the DateFormat once the JS object that
  // holds the pointer gets garbage collected.
  static void DeleteDateTimeFormat(v8::Persistent<v8::Value> object,
                                   void* param);

  // Formats date and returns corresponding string.
  static v8::Handle<v8::Value> Format(const v8::Arguments& args);

  // All date time symbol methods below return stand-alone names in
  // either narrow, abbreviated or wide width.

  // Get list of months.
  static v8::Handle<v8::Value> GetMonths(const v8::Arguments& args);

  // Get list of weekdays.
  static v8::Handle<v8::Value> GetWeekdays(const v8::Arguments& args);

  // Get list of eras.
  static v8::Handle<v8::Value> GetEras(const v8::Arguments& args);

  // Get list of day periods.
  static v8::Handle<v8::Value> GetAmPm(const v8::Arguments& args);

 private:
  DateTimeFormat();

  static v8::Persistent<v8::FunctionTemplate> datetime_format_template_;
};

}  // namespace v8_i18n

#endif  // V8_I18N_SRC_DATETIME_FORMAT_H_
