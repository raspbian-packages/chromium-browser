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

//
// WARNING: This API is deprecated and will soon be replaced by ECMAScript
// Globalization API as soon as that specification becomes stable.
//

/**
 * v8Locale class is an aggregate class of all i18n API calls.
 * @param {string} localeID - Unicode identifier of the locale to create
 *   v8Locale from. See http://unicode.org/reports/tr35/#BCP_47_Conformance
 * @constructor
 */
v8Locale = function(localeID) {
  native function NativeJSLocale();
  var properties;

  // Make sure we don't pollute the global object if we are called without 'new'
  if (!(this instanceof v8Locale)) {
    return new v8Locale(localeID);
  }

  if (typeof(localeID) !== 'string') {
    localeID = 'root';
  }

  properties = NativeJSLocale({'localeID': localeID});

  // Keep the resolved ICU locale ID around to avoid resolving localeID to
  // ICU locale ID every time BreakIterator, Collator and so forth are called.
  this.__icuLocaleID = properties.icuLocaleID;
  this.options = {'localeID': properties.localeID};

  return this;
};

/**
 * v8BreakIterator class implements locale aware segmenatation.
 * It is not part of ECMAScript proposal.
 * @param {Object} locale - locale object to pass to break
 *   iterator implementation.
 * @param {string} type - type of segmenatation:
 *   - character
 *   - word
 *   - sentence
 *   - line
 * @private
 * @constructor
 */
v8Locale.v8BreakIterator = function(locale, type) {
  native function NativeJSBreakIterator();

  locale = v8Locale.__createLocaleOrDefault(locale);
  // BCP47 ID would work in this case, but we use ICU locale for consistency.
  return NativeJSBreakIterator(locale.__icuLocaleID, type);
};

/**
 * Type of the break we encountered during previous iteration.
 * @type{Enum}
 */
v8Locale.v8BreakIterator.BreakType = {
  'unknown': -1,
  'none': 0,
  'number': 100,
  'word': 200,
  'kana': 300,
  'ideo': 400
};

/**
 * Creates new v8BreakIterator based on current locale.
 * @param {string} - type of segmentation. See constructor.
 * @returns {Object} - new v8BreakIterator object.
 */
v8Locale.prototype.v8CreateBreakIterator = function(type) {
  return v8Locale.v8BreakIterator(this, type);
};

/**
 * Collator class implements locale-aware sort.
 * @param {Object} locale - locale object to pass to collator implementation.
 * @param {Object} settings - collation flags:
 *   - ignoreCase
 *   - ignoreAccents
 *   - numeric
 * @private
 * @constructor
 */
v8Locale.Collator = function(locale, settings) {
  native function NativeJSCollator();

  var cleanSettings = v8Locale.__copyOptions(
    {}, settings,
    ['ignoreCase', 'ignoreAccents', 'numeric']
  );

  locale = v8Locale.__createLocaleOrDefault(locale);
  return NativeJSCollator(locale.__icuLocaleID, cleanSettings);
};

/**
 * Creates new Collator based on current locale.
 * @param {Object} - collation flags. See constructor.
 * @returns {Object} - new Collator object.
 */
v8Locale.prototype.createCollator = function(settings) {
  return v8Locale.Collator(this, settings);
};

/**
 * DateTimeFormat class implements locale-aware date and time formatting.
 * Constructor is not part of public API.
 * @param {Object} locale - locale object to pass to formatter.
 * @param {Object} settings - formatting flags:
 *   - skeleton
 *   - dateType
 * @private
 * @constructor
 */
v8Locale.__DateTimeFormat = function(locale, settings) {
  native function NativeJSDateTimeFormat();

  var cleanSettings;
  var validTypes = /^(short|medium|long|full)$/;

  cleanSettings = v8Locale.__copyOptions({}, settings, 'skeleton');
  if (cleanSettings.skeleton === undefined) {
    if (settings && settings.dateType && validTypes.test(settings.dateType)) {
      cleanSettings.dateStyle = settings.dateType;
    } else {
      // Default is to show short date and time.
      cleanSettings.dateStyle = 'short';
      cleanSettings.timeStyle = 'short';
    }
  }

  locale = v8Locale.__createLocaleOrDefault(locale);
  return NativeJSDateTimeFormat(locale.__icuLocaleID, cleanSettings);
};

/**
 * Creates new DateTimeFormat based on current locale.
 * @param {Object} - formatting flags. See constructor.
 * @returns {Object} - new DateTimeFormat object.
 */
v8Locale.prototype.createDateTimeFormat = function(settings) {
  return v8Locale.__DateTimeFormat(this, settings);
};

/**
 * Add values of listed options to the target object, if the values exist and
 * are strings.
 * @param {Object} targetObject - target object.
 * @param {Object} sourceObject - source object.
 * @param {Array.<string>|string} optionList - an option name or a list of
 *   option names.
 * @returns {Object} - reference to potentially modified targetObject.
 * @private
 */
v8Locale.__copyOptions = function(targetObject, sourceObject, optionList) {
  var i, option, value;

  if (!sourceObject) {
    return targetObject;
  }

  if (typeof(optionList) === 'string') {
    optionList = [optionList];
  }

  for (i = 0; i < optionList.length; i += 1) {
    option = optionList[i];
    value = sourceObject[option];
    if (typeof(value) === 'string') {
      targetObject[option] = value;
    }
  }

  return targetObject;
};

/**
 * If locale is valid (defined and of v8Locale type) we return it. If not
 * we create default locale and return it.
 * @param {!Object} locale - user provided locale.
 * @returns {Object} - v8Locale object.
 * @private
 */
v8Locale.__createLocaleOrDefault = function(locale) {
  if (!locale || !(locale instanceof v8Locale)) {
    return new v8Locale();
  } else {
    return locale;
  }
};
