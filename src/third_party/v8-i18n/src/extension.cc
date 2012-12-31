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

#include "include/extension.h"

#include "src/break-iterator.h"
#include "src/collator.h"
#include "src/datetime-format.h"
#include "src/intl-break-iterator.h"
#include "src/intl-collator.h"
#include "src/intl-date-format.h"
#include "src/intl-number-format.h"
#include "src/locale-list.h"
#include "src/locale.h"
#include "src/natives.h"
#include "src/number-format.h"

namespace v8_i18n {

Extension* Extension::extension_ = NULL;

Extension::Extension()
    : v8::Extension("v8/i18n", Natives::GetScriptSource()) {
}

v8::Handle<v8::FunctionTemplate> Extension::GetNativeFunction(
    v8::Handle<v8::String> name) {
  if (name->Equals(v8::String::New("NativeJSLocale"))) {
    return v8::FunctionTemplate::New(Locale::JSLocale);
  } else if (name->Equals(v8::String::New("NativeJSBreakIterator"))) {
    return v8::FunctionTemplate::New(BreakIterator::JSBreakIterator);
  } else if (name->Equals(v8::String::New("NativeJSCollator"))) {
    return v8::FunctionTemplate::New(Collator::JSCollator);
  } else if (name->Equals(v8::String::New("NativeJSDateTimeFormat"))) {
    return v8::FunctionTemplate::New(DateTimeFormat::JSDateTimeFormat);
  } else if (name->Equals(v8::String::New("NativeJSNumberFormat"))) {
    return v8::FunctionTemplate::New(NumberFormat::JSNumberFormat);
  } else if (name->Equals(v8::String::New("NativeJSCanonicalizeLanguageTag"))) {
    return v8::FunctionTemplate::New(JSCanonicalizeLanguageTag);
  } else if (name->Equals(v8::String::New("NativeJSAvailableLocalesOf"))) {
    return v8::FunctionTemplate::New(JSAvailableLocalesOf);
  } else if (name->Equals(v8::String::New("NativeJSCreateDateTimeFormat"))) {
    return v8::FunctionTemplate::New(IntlDateFormat::JSCreateDateTimeFormat);
  } else if (name->Equals(v8::String::New("NativeJSInternalDateFormat"))) {
    return v8::FunctionTemplate::New(IntlDateFormat::JSInternalFormat);
  } else if (name->Equals(v8::String::New("NativeJSCreateNumberFormat"))) {
    return v8::FunctionTemplate::New(IntlNumberFormat::JSCreateNumberFormat);
  } else if (name->Equals(v8::String::New("NativeJSInternalNumberFormat"))) {
    return v8::FunctionTemplate::New(IntlNumberFormat::JSInternalFormat);
  } else if (name->Equals(v8::String::New("NativeJSCreateCollator"))) {
    return v8::FunctionTemplate::New(IntlCollator::JSCreateCollator);
  } else if (name->Equals(v8::String::New("NativeJSInternalCompare"))) {
    return v8::FunctionTemplate::New(IntlCollator::JSInternalCompare);
  } else if (name->Equals(v8::String::New("NativeJSCreateBreakIterator"))) {
    return v8::FunctionTemplate::New(IntlBreakIterator::JSCreateBreakIterator);
  } else if (name->Equals(v8::String::New("NativeJSBreakIteratorAdoptText"))) {
    return v8::FunctionTemplate::New(
	IntlBreakIterator::JSInternalBreakIteratorAdoptText);
  } else if (name->Equals(v8::String::New("NativeJSBreakIteratorFirst"))) {
    return v8::FunctionTemplate::New(
        IntlBreakIterator::JSInternalBreakIteratorFirst);
  } else if (name->Equals(v8::String::New("NativeJSBreakIteratorNext"))) {
    return v8::FunctionTemplate::New(
        IntlBreakIterator::JSInternalBreakIteratorNext);
  } else if (name->Equals(v8::String::New("NativeJSBreakIteratorCurrent"))) {
    return v8::FunctionTemplate::New(
        IntlBreakIterator::JSInternalBreakIteratorCurrent);
  } else if (name->Equals(v8::String::New("NativeJSBreakIteratorBreakType"))) {
    return v8::FunctionTemplate::New(
	IntlBreakIterator::JSInternalBreakIteratorBreakType);
  }

  return v8::Handle<v8::FunctionTemplate>();
}

Extension* Extension::get() {
  if (!extension_) {
    extension_ = new Extension();
  }
  return extension_;
}

void Extension::Register() {
  static v8::DeclareExtension extension_declaration(Extension::get());
}

}  // namespace v8_i18n
