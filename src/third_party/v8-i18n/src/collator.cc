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

#include "src/collator.h"

#include "unicode/coll.h"
#include "unicode/locid.h"
#include "unicode/ucol.h"

namespace v8_i18n {

v8::Persistent<v8::FunctionTemplate> Collator::collator_template_;

icu::Collator* Collator::UnpackCollator(v8::Handle<v8::Object> obj) {
  if (collator_template_->HasInstance(obj)) {
    return static_cast<icu::Collator*>(obj->GetPointerFromInternalField(0));
  }

  return NULL;
}

void Collator::DeleteCollator(v8::Persistent<v8::Value> object, void* param) {
  v8::Persistent<v8::Object> persistent_object =
      v8::Persistent<v8::Object>::Cast(object);

  // First delete the hidden C++ object.
  // Unpacking should never return NULL here. That would only happen if
  // this method is used as the weak callback for persistent handles not
  // pointing to a collator.
  delete UnpackCollator(persistent_object);

  // Then dispose of the persistent handle to JS object.
  persistent_object.Dispose();
}

// Throws a JavaScript exception.
static v8::Handle<v8::Value> ThrowUnexpectedObjectError() {
  // Returns undefined, and schedules an exception to be thrown.
  return v8::ThrowException(v8::Exception::Error(
      v8::String::New("Collator method called on an object "
                      "that is not a Collator.")));
}

// Extract a boolean option named in |option| and set it to |result|.
// Return true if it's specified. Otherwise, return false.
static bool ExtractBooleanOption(const v8::Local<v8::Object>& options,
                                 const char* option,
                                 bool* result) {
  v8::HandleScope handle_scope;
  v8::TryCatch try_catch;
  v8::Handle<v8::Value> value = options->Get(v8::String::New(option));
  if (try_catch.HasCaught()) {
    return false;
  }
  // No need to check if |value| is empty because it's taken care of
  // by TryCatch above.
  if (!value->IsUndefined() && !value->IsNull()) {
    if (value->IsBoolean()) {
      *result = value->BooleanValue();
      return true;
    }
  }
  return false;
}

// When there's an ICU error, throw a JavaScript error with |message|.
static v8::Handle<v8::Value> ThrowExceptionForICUError(const char* message) {
  return v8::ThrowException(v8::Exception::Error(v8::String::New(message)));
}

v8::Handle<v8::Value> Collator::CollatorCompare(const v8::Arguments& args) {
  if (args.Length() != 2 || !args[0]->IsString() || !args[1]->IsString()) {
    return v8::ThrowException(v8::Exception::SyntaxError(
        v8::String::New("Two string arguments are required.")));
  }

  icu::Collator* collator = UnpackCollator(args.Holder());
  if (!collator) {
    return ThrowUnexpectedObjectError();
  }

  v8::String::Value string_value1(args[0]);
  v8::String::Value string_value2(args[1]);
  const UChar* string1 = reinterpret_cast<const UChar*>(*string_value1);
  const UChar* string2 = reinterpret_cast<const UChar*>(*string_value2);
  UErrorCode status = U_ZERO_ERROR;
  UCollationResult result = collator->compare(
      string1, string_value1.length(), string2, string_value2.length(), status);

  if (U_FAILURE(status)) {
    return ThrowExceptionForICUError(
        "Unexpected failure in Collator.compare.");
  }

  return v8::Int32::New(result);
}

v8::Handle<v8::Value> Collator::JSCollator(const v8::Arguments& args) {
  v8::HandleScope handle_scope;

  if (args.Length() != 2 || !args[0]->IsString() || !args[1]->IsObject()) {
    return v8::ThrowException(v8::Exception::SyntaxError(
        v8::String::New("Locale and collation options are required.")));
  }

  v8::String::AsciiValue locale(args[0]);
  icu::Locale icu_locale(*locale);

  icu::Collator* collator = NULL;
  UErrorCode status = U_ZERO_ERROR;
  collator = icu::Collator::createInstance(icu_locale, status);

  if (U_FAILURE(status)) {
    delete collator;
    return ThrowExceptionForICUError("Failed to create collator.");
  }

  v8::Local<v8::Object> options(args[1]->ToObject());

  // Below, we change collation options that are explicitly specified
  // by a caller in JavaScript. Otherwise, we don't touch because
  // we don't want to change the locale-dependent default value.
  // The three options below are very likely to have the same default
  // across locales, but I haven't checked them all. Others we may add
  // in the future have certainly locale-dependent default (e.g.
  // caseFirst is upperFirst for Danish while is off for most other locales).

  bool ignore_case, ignore_accents, numeric;

  if (ExtractBooleanOption(options, "ignoreCase", &ignore_case)) {
    // We need to explicitly set the level to secondary to get case ignored.
    // The default L3 ignores UCOL_CASE_LEVEL == UCOL_OFF !
    if (ignore_case) {
      collator->setStrength(icu::Collator::SECONDARY);
    }
    collator->setAttribute(UCOL_CASE_LEVEL, ignore_case ? UCOL_OFF : UCOL_ON,
                           status);
    if (U_FAILURE(status)) {
      delete collator;
      return ThrowExceptionForICUError("Failed to set ignoreCase.");
    }
  }

  // Accents are taken into account with strength secondary or higher.
  if (ExtractBooleanOption(options, "ignoreAccents", &ignore_accents)) {
    if (!ignore_accents) {
      collator->setStrength(icu::Collator::SECONDARY);
    } else {
      collator->setStrength(icu::Collator::PRIMARY);
    }
  }

  if (ExtractBooleanOption(options, "numeric", &numeric)) {
    collator->setAttribute(UCOL_NUMERIC_COLLATION,
                           numeric ? UCOL_ON : UCOL_OFF, status);
    if (U_FAILURE(status)) {
      delete collator;
      return ThrowExceptionForICUError("Failed to set numeric sort option.");
    }
  }

  if (collator_template_.IsEmpty()) {
    v8::Local<v8::FunctionTemplate> raw_template(v8::FunctionTemplate::New());
    raw_template->SetClassName(v8::String::New("v8Locale.Collator"));

    // Define internal field count on instance template.
    v8::Local<v8::ObjectTemplate> object_template =
        raw_template->InstanceTemplate();

    // Set aside internal fields for icu collator.
    object_template->SetInternalFieldCount(1);

    // Define all of the prototype methods on prototype template.
    v8::Local<v8::ObjectTemplate> proto = raw_template->PrototypeTemplate();
    proto->Set(v8::String::New("compare"),
               v8::FunctionTemplate::New(CollatorCompare));

    collator_template_ =
        v8::Persistent<v8::FunctionTemplate>::New(raw_template);
  }

  // Create an empty object wrapper.
  v8::Local<v8::Object> local_object =
      collator_template_->GetFunction()->NewInstance();
  v8::Persistent<v8::Object> wrapper =
      v8::Persistent<v8::Object>::New(local_object);

  // Set collator as internal field of the resulting JS object.
  wrapper->SetPointerInInternalField(0, collator);

  // Make object handle weak so we can delete iterator once GC kicks in.
  wrapper.MakeWeak(NULL, DeleteCollator);

  return wrapper;
}

}  // namespace v8_i18n
