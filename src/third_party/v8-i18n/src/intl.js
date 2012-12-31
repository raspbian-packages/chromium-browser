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

/**
 * Intl object is a single object that has some named properties,
 * all of which are constructors.
 */
var v8Intl = (function() {

var v8Intl = {};

/**
 * Global native (C++) methods.
 */
native function NativeJSAvailableLocalesOf();

/**
 * List of available services.
 */
var AVAILABLE_SERVICES = ['collator',
			  'numberformat',
			  'dateformat',
			  'breakiterator'];

/**
 * Caches available locales for each service.
 */
var AVAILABLE_LOCALES = {
  'collator': undefined,
  'numberformat': undefined,
  'dateformat': undefined,
  'breakiterator': undefined
};

/**
 * Unicode extension regular expression.
 */
var UNICODE_EXTENSION_RE = new RegExp('-u(-[a-z0-9]{2,8})+', 'g');

/**
 * Matches any Unicode extension.
 */
var ANY_EXTENSION_RE = new RegExp('-[a-z0-9]{1}-.*', 'g');

/**
 * Replace quoted text (single quote, anything but the quote and quote again).
 */
var QUOTED_STRING_RE = new RegExp("'[^']+'", 'g');

/**
 * Matches valid service name.
 */
var SERVICE_RE =
    new RegExp('^(collator|numberformat|dateformat|breakiterator)$');

/**
 * Maps ICU calendar names into LDML type.
 */
var ICU_CALENDAR_MAP = {
  'gregorian': 'gregory',
  'japanese': 'japanese',
  'buddhist': 'buddhist',
  'roc': 'roc',
  'persian': 'persian',
  'islamic-civil': 'islamicc',
  'islamic': 'islamic',
  'hebrew': 'hebrew',
  'chinese': 'chinese',
  'indian': 'indian',
  'coptic': 'coptic',
  'ethiopic': 'ethiopic',
  'ethiopic-amete-alem': 'ethioaa'
};

/**
 * Map of Unicode extensions to option properties, and their values and types,
 * for a collator.
 */
var COLLATOR_KEY_MAP = {
  'kn': {'property': 'numeric', 'type': 'boolean'},
  'kk': {'property': 'normalization', 'type':'boolean'},
  'kf': {'property': 'caseFirst', 'type': 'string',
         'values': ['false', 'lower', 'upper']}
};

/**
 * Map of Unicode extensions to option properties, and their values and types,
 * for a number format.
 */
var NUMBER_FORMAT_KEY_MAP = {
  'nu': {'property': undefined, 'type': 'string'}
};

/**
 * Map of Unicode extensions to option properties, and their values and types,
 * for a date/time format.
 */
var DATETIME_FORMAT_KEY_MAP = {
  'ca': {'property': undefined, 'type': 'string'},
  'nu': {'property': undefined, 'type': 'string'}
};


/**
 * Canonicalizes the language tag, or throws in case the tag is invalid.
 */
function canonicalizeLanguageTag(localeID) {
  native function NativeJSCanonicalizeLanguageTag();

  if (typeof localeID !== 'string' && typeof localeID !== 'object') {
    throw new TypeError('Language ID should be string or object.');
  }

  // This call will strip -kn but not -kn-true extensions.
  // ICU bug filled - http://bugs.icu-project.org/trac/ticket/9265.
  // TODO(cira): check if -u-kn-true-kc-true-kh-true still throws after
  // upgrade to ICU 4.9.
  var tag = NativeJSCanonicalizeLanguageTag(String(localeID));
  if (tag === 'invalid-tag') {
    throw new RangeError('Invalid language tag: ' + localeID);
  }

  return tag;
}


/**
 * Returns an array where all locales are canonicalized and duplicates removed.
 * Throws on locales that are not well formed BCP47 tags.
 */
function initializeLocaleList(locales) {
  var seen = [];
  if (locales === undefined) {
    // Constructor is called without arguments.
    seen = [];
  } else {
    var o = toObject(locales);
    // Converts it to UInt32 (>>> is shr on 32bit integers).
    var len = o.length >>> 0;

    for (var k = 0; k < len; k++) {
      if (k in o) {
        var value = o[k];

        var tag = canonicalizeLanguageTag(value);

        if (seen.indexOf(tag) === -1) {
          seen.push(tag);
        }
      }
    }
  }

  return seen;
}


/**
 * Initializes the given object so it's a valid Collator instance.
 * Useful for subclassing.
 */
function initializeCollator(collator, locales, options) {
  native function NativeJSCreateCollator();

  if (options === undefined) {
    options = {};
  }

  var getOption = getGetOption(options, 'collator');

  var internalOptions = {};

  internalOptions.usage =
      getOption('usage', 'string', ['sort', 'search'], 'sort');

  internalOptions.sensitivity =
      getOption('sensitivity', 'string', ['base', 'accent', 'case', 'variant']);
  if (internalOptions.sensitivity === undefined &&
      internalOptions.usage === 'sort') {
    internalOptions.sensitivity = 'variant';
  }

  internalOptions.ignorePunctuation =
      getOption('ignorePunctuation', 'boolean', undefined, false);

  var locale = resolveLocale('collator', locales, options);

  // ICU can't take kb, kc... parameters through localeID, so we need to pass
  // them as options.
  // One exception is -co- which has to be part of the extension, but only for
  // usage: sort, and its value can't be 'standard' or 'search'.
  var extensionMap = parseExtension(locale.extension);
  setOptions(
      options, extensionMap, COLLATOR_KEY_MAP, getOption, internalOptions);

  internalOptions.collation = 'default';
  var extension = '';
  if (extensionMap.hasOwnProperty('co') && internalOptions.usage === 'sort') {
    if (extensionMap.co &&
        extensionMap.co !== 'standard' && extensionMap.co !== 'search') {
      extension = '-u-co-' + extensionMap.co;
      // ICU can't tell us what the collation is, so save user's input.
      internalOptions.collation = extensionMap.co;
    }
  } else if (internalOptions.usage === 'search') {
    extension = '-u-co-search';
  }

  var internalCollator = NativeJSCreateCollator(locale.locale + extension,
						internalOptions);
  internalCollator.usage = internalOptions.usage;
  internalCollator.collation = internalOptions.collation;

  // Writable, configurable and enumerable are set to false by default.
  Object.defineProperty(collator, 'collator', {value: internalCollator});

  return collator;
}


/**
 * Constructs v8Intl.Collator object given optional locales and options
 * parameters.
 *
 * @constructor
 */
v8Intl.Collator = function(locales, options) {
  if (!this || this === v8Intl) {
    // Constructor is called as a function.
    return new v8Intl.Collator(locales, options);
  }

  return initializeCollator(toObject(this), locales, options);
};


/**
 * Collator resolvedOptions method.
 */
v8Intl.Collator.prototype.resolvedOptions = function() {
  return {
    locale: this.collator.locale,
    usage: this.collator.usage,
    sensitivity: this.collator.sensitivity,
    ignorePunctuation: this.collator.ignorePunctuation,
    numeric: this.collator.numeric,
    normalization: this.collator.normalization,
    caseFirst: this.collator.caseFirst,
    collation: this.collator.collation
  };
};


/**
 * Returns the subset of the given locale list for which this locale list
 * has a matching (possibly fallback) locale. Locales appear in the same
 * order in the returned list as in the input list.
 */
v8Intl.Collator.supportedLocalesOf = function(locales, options) {
  return supportedLocalesOf('collator', locales, options);
};


/**
 * When the compare method is called with two arguments x and y, it returns a
 * Number other than NaN that represents the result of a locale-sensitive
 * String comparison of x with y.
 * The result is intended to order String values in the sort order specified
 * by the effective locale and collation options computed during construction
 * of this Collator object, and will be negative, zero, or positive, depending
 * on whether x comes before y in the sort order, the Strings are equal under
 * the sort order, or x comes after y in the sort order, respectively.
 */
function compare(collator, x, y) {
  native function NativeJSInternalCompare();
  return NativeJSInternalCompare(collator.collator, String(x), String(y));
};


addBoundMethod(v8Intl.Collator, 'compare', compare);


/**
 * Verifies that the input is a well-formed ISO 4217 currency code.
 */
function isWellFormedCurrencyCode(currency) {
  if (typeof currency !== "string") {
    return false;
  }

  var code = String(currency).toUpperCase();
  if (code.length !== 3) {
    return false;
  }

  if (code.match(/[^A-Z]/) !== null) {
    return false;
  }
  return true;
}


/**
 * Initializes the given object so it's a valid NumberFormat instance.
 * Useful for subclassing.
 */
function initializeNumberFormat(numberFormat, locales, options) {
  native function NativeJSCreateNumberFormat();

  if (options === undefined) {
    options = {};
  }

  var getOption = getGetOption(options, 'numberformat');

  var locale = resolveLocale('numberformat', locales, options);

  var internalOptions = {};
  internalOptions.style = getOption(
      'style', 'string', ['decimal', 'percent', 'currency'], 'decimal');

  var currency = getOption('currency', 'string');
  if (currency && !isWellFormedCurrencyCode(currency)) {
    throw new RangeError('Invalid currency code: ' + currency);
  }

  if (internalOptions.style === 'currency' && currency === undefined) {
    throw new TypeError('Currency code is required with currency style.');
  }

  var currencyDisplay = getOption(
      'currencyDisplay', 'string', ['code', 'symbol', 'name'], 'symbol');
  if (internalOptions.style === 'currency') {
    internalOptions.currency = currency.toUpperCase();
    internalOptions.currencyDisplay = currencyDisplay;
  }

  var digitRanges = ['minimumIntegerDigits', 'minimumFractionDigits',
                     'maximumFractionDigits', 'minimumSignificantDigits',
                     'maximumSignificantDigits'];
  for (var i = 0; i < digitRanges.length; ++i) {
    var digits = options[digitRanges[i]];
    if (digits !== undefined && (digits >= 0 && digits <= 21)) {
      internalOptions[digitRanges[i]] = Number(digits);
    }
  }

  internalOptions.useGrouping = getOption(
      'useGrouping', 'boolean', undefined, true);

  // ICU prefers options to be passed using -u- extension key/values for
  // number format, so we need to build that.
  var extensionMap = parseExtension(locale.extension);
  var extension = setOptions(options, extensionMap, NUMBER_FORMAT_KEY_MAP,
                             getOption, internalOptions);

  var formatter = NativeJSCreateNumberFormat(locale.locale + extension,
                                             internalOptions);

  // We can't get information about number or currency style from ICU, so we
  // assume user request was fulfilled.
  formatter.style = internalOptions.style;
  if (internalOptions.style === 'currency') {
    formatter.currencyDisplay = currencyDisplay;
  }

  Object.defineProperty(numberFormat, 'formatter', {value: formatter});

  return numberFormat;
}


/**
 * Constructs v8Intl.NumberFormat object given optional locales and options
 * parameters.
 *
 * @constructor
 */
v8Intl.NumberFormat = function(locales, options) {
  if (!this || this === v8Intl) {
    // Constructor is called as a function.
    return new v8Intl.NumberFormat(locales, options);
  }

  return initializeNumberFormat(toObject(this), locales, options);
};


/**
 * NumberFormat resolvedOptions method.
 */
v8Intl.NumberFormat.prototype.resolvedOptions = function() {
  return {
    locale: this.formatter.locale,
    numberingSystem: this.formatter.numberingSystem,
    style: this.formatter.style,
    currency: this.formatter.currency,
    currencyDisplay: this.formatter.currencyDisplay,
    useGrouping: this.formatter.useGrouping,
    minimumIntegerDigits: this.formatter.minimumIntegerDigits,
    minimumFractionDigits: this.formatter.minimumFractionDigits,
    maximumFractionDigits: this.formatter.maximumFractionDigits,
    minimumSignificantDigits: this.formatter.minimumSignificantDigits,
    maximumSignificantDigits: this.formatter.maximumSignificantDigits
  };
};


/**
 * Returns the subset of the given locale list for which this locale list
 * has a matching (possibly fallback) locale. Locales appear in the same
 * order in the returned list as in the input list.
 */
v8Intl.NumberFormat.supportedLocalesOf = function(locales, options) {
  return supportedLocalesOf('numberformat', locales, options);
};


/**
 * Returns a String value representing the result of calling ToNumber(value)
 * according to the effective locale and the formatting options of this
 * NumberFormat.
 */
function formatNumber(formatter, value) {
  native function NativeJSInternalNumberFormat();

  return NativeJSInternalNumberFormat(formatter.formatter, Number(value));
}


addBoundMethod(v8Intl.NumberFormat, 'format', formatNumber);


/**
 * Returns a string that matches LDML representation of the options object.
 */
function toLDMLString(options) {
  var getOption = getGetOption(options, 'dateformat');

  var ldmlString = '';

  var option = getOption('weekday', 'string', ['narrow', 'short', 'long']);
  ldmlString += appendToLDMLString(
      option, {narrow: 'EEEEE', short: 'EEE', long: 'EEEE'});

  option = getOption('era', 'string', ['narrow', 'short', 'long']);
  ldmlString += appendToLDMLString(
      option, {narrow: 'GGGGG', short: 'GGG', long: 'GGGG'});

  option = getOption('year', 'string', ['2-digit', 'numeric']);
  ldmlString += appendToLDMLString(option, {'2-digit': 'yy', 'numeric': 'y'});

  option = getOption('month', 'string',
                     ['2-digit', 'numeric', 'narrow', 'short', 'long']);
  ldmlString += appendToLDMLString(option, {'2-digit': 'MM', 'numeric': 'M',
          'narrow': 'MMMMM', 'short': 'MMM', 'long': 'MMMM'});

  option = getOption('day', 'string', ['2-digit', 'numeric']);
  ldmlString += appendToLDMLString(
      option, {'2-digit': 'dd', 'numeric': 'd'});

  var hr12 = getOption('hour12', 'boolean');
  option = getOption('hour', 'string', ['2-digit', 'numeric']);
  if (hr12 === undefined) {
    ldmlString += appendToLDMLString(option, {'2-digit': 'jj', 'numeric': 'j'});
  } else if (hr12 === true) {
    ldmlString += appendToLDMLString(option, {'2-digit': 'hh', 'numeric': 'h'});
  } else {
    ldmlString += appendToLDMLString(option, {'2-digit': 'HH', 'numeric': 'H'});
  }

  option = getOption('minute', 'string', ['2-digit', 'numeric']);
  ldmlString += appendToLDMLString(option, {'2-digit': 'mm', 'numeric': 'm'});

  option = getOption('second', 'string', ['2-digit', 'numeric']);
  ldmlString += appendToLDMLString(option, {'2-digit': 'ss', 'numeric': 's'});

  option = getOption('timeZoneName', 'string', ['short', 'long']);
  ldmlString += appendToLDMLString(option, {short: 'v', long: 'vv'});

  return ldmlString;
}


/**
 * Returns either LDML equivalent of the current option or empty string.
 */
function appendToLDMLString(option, pairs) {
  if (option !== undefined) {
    return pairs[option];
  } else {
    return '';
  }
}


/**
 * Returns object that matches LDML representation of the date.
 */
function fromLDMLString(ldmlString) {
  // First remove '' quoted text, so we lose 'Uhr' strings.
  ldmlString = ldmlString.replace(QUOTED_STRING_RE, '');

  var options = {};
  var match = ldmlString.match(/E{3,5}/g);
  options = appendToDateTimeObject(
      options, 'weekday', match, {EEEEE: 'narrow', EEE: 'short', EEEE: 'long'});

  match = ldmlString.match(/G{3,5}/g);
  options = appendToDateTimeObject(
      options, 'era', match, {GGGGG: 'narrow', GGG: 'short', GGGG: 'long'});

  match = ldmlString.match(/y{1,2}/g);
  options = appendToDateTimeObject(
      options, 'year', match, {y: 'numeric', yy: '2-digit'});

  match = ldmlString.match(/M{1,5}/g);
  options = appendToDateTimeObject(options, 'month', match, {MM: '2-digit',
      M: 'numeric', MMMMM: 'narrow', MMM: 'short', MMMM: 'long'});

  // Sometimes we get L instead of M for month - standalone name.
  match = ldmlString.match(/L{1,5}/g);
  options = appendToDateTimeObject(options, 'month', match, {LL: '2-digit',
      L: 'numeric', LLLLL: 'narrow', LLL: 'short', LLLL: 'long'});

  match = ldmlString.match(/d{1,2}/g);
  options = appendToDateTimeObject(
      options, 'day', match, {d: 'numeric', dd: '2-digit'});

  match = ldmlString.match(/h{1,2}/g);
  if (match !== null) {
    options['hour12'] = true;
  }
  options = appendToDateTimeObject(
      options, 'hour', match, {h: 'numeric', hh: '2-digit'});

  match = ldmlString.match(/H{1,2}/g);
  if (match !== null) {
    options['hour12'] = false;
  }
  options = appendToDateTimeObject(
      options, 'hour', match, {H: 'numeric', HH: '2-digit'});

  match = ldmlString.match(/m{1,2}/g);
  options = appendToDateTimeObject(
      options, 'minute', match, {m: 'numeric', mm: '2-digit'});

  match = ldmlString.match(/s{1,2}/g);
  options = appendToDateTimeObject(
      options, 'second', match, {s: 'numeric', ss: '2-digit'});

  match = ldmlString.match(/v{1,2}/g);
  options = appendToDateTimeObject(
      options, 'timeZoneName', match, {v: 'short', vv: 'long'});

  return options;
}


function appendToDateTimeObject(options, option, match, pairs) {
  if (match === null) {
    if (!options.hasOwnProperty(option)) {
      options[option] = undefined;
    }
    return options;
  }

  var property = match[0];
  options[option] = pairs[property];

  return options;
}


/**
 * Returns options with at least default values in it.
 */
function toDateTimeOptions(options, required, defaults) {
  if (options === undefined) {
    options = null;
  } else {
    options = toObject(options);
  }

  options = Object.apply(this, [options]);

  var needsDefault = true;
  if ((required === 'date' || required === 'all') &&
      (options.weekday !== undefined || options.year !== undefined ||
       options.month !== undefined || options.day !== undefined)) {
    needsDefault = false;
  }

  if ((required === 'time' || required === 'all') &&
      (options.hour !== undefined || options.minute !== undefined ||
       options.second !== undefined)) {
    needsDefault = false;
  }

  if (needsDefault && (defaults === 'date' || defaults === 'all')) {
    Object.defineProperty(options, 'year', {value: 'numeric',
                                            writable: true,
                                            enumerable: true,
                                            configurable: true});
    Object.defineProperty(options, 'month', {value: 'numeric',
                                             writable: true,
                                             enumerable: true,
                                             configurable: true});
    Object.defineProperty(options, 'day', {value: 'numeric',
                                           writable: true,
                                           enumerable: true,
                                           configurable: true});
  }

  if (needsDefault && (defaults === 'time' || defaults === 'all')) {
    Object.defineProperty(options, 'hour', {value: 'numeric',
                                            writable: true,
                                            enumerable: true,
                                            configurable: true});
    Object.defineProperty(options, 'minute', {value: 'numeric',
                                              writable: true,
                                              enumerable: true,
                                              configurable: true});
    Object.defineProperty(options, 'second', {value: 'numeric',
                                              writable: true,
                                              enumerable: true,
                                              configurable: true});
  }

  return options;
}


/**
 * Initializes the given object so it's a valid DateTimeFormat instance.
 * Useful for subclassing.
 */
function initializeDateTimeFormat(dateFormat, locales, options) {
  native function NativeJSCreateDateTimeFormat();

  if (options === undefined) {
    options = {};
  }

  var locale = resolveLocale('dateformat', locales, options);

  options = toDateTimeOptions(options, 'all', 'date');

  var getOption = getGetOption(options, 'dateformat');

  // We implement only best fit algorithm, but still need to check
  // if the formatMatcher values are in range.
  var matcher = getOption('formatMatcher', 'string',
                          ['basic', 'best fit'], 'best fit');

  // Build LDML string for the skeleton that we pass to the formatter.
  var ldmlString = toLDMLString(options);

  // Filter out supported extension keys so we know what to put in resolved
  // section later on.
  // We need to pass calendar and number system to the method.
  var tz = options.timeZone;
  if (tz !== undefined) {
    tz = String(tz).toUpperCase();
    if (tz !== "UTC") {
      throw new RangeError("Invalid time zone specified: " + tz);
    }
  }

  // ICU prefers options to be passed using -u- extension key/values, so
  // we need to build that.
  var internalOptions = {};
  var extensionMap = parseExtension(locale.extension);
  var extension = setOptions(options, extensionMap, DATETIME_FORMAT_KEY_MAP,
                             getOption, internalOptions);

  var formatter = NativeJSCreateDateTimeFormat(
      locale.locale + extension, {skeleton: ldmlString, timeZone: tz});

  formatter.tz = tz;

  Object.defineProperty(dateFormat, 'formatter', {value: formatter});

  return dateFormat;
}


/**
 * Constructs v8Intl.DateTimeFormat object given optional locales and options
 * parameters.
 *
 * @constructor
 */
v8Intl.DateTimeFormat = function(locales, options) {
  if (!this || this === v8Intl) {
    // Constructor is called as a function.
    return new v8Intl.DateTimeFormat(locales, options);
  }

  return initializeDateTimeFormat(toObject(this), locales, options);
};


/**
 * DateTimeFormat resolvedOptions method.
 */
v8Intl.DateTimeFormat.prototype.resolvedOptions = function() {
  var fromPattern = fromLDMLString(this.formatter.pattern);
  var userCalendar = ICU_CALENDAR_MAP[this.formatter.calendar];
  if (userCalendar === undefined) {
    // Use ICU name if we don't have a match. It shouldn't happen, but
    // it would be too strict to throw for this.
    userCalendar = this.formatter.calendar;
  }

  return {
    locale: this.formatter.locale,
    numberingSystem: this.formatter.numberingSystem,
    calendar: userCalendar,
    timeZone: this.formatter.tz,
    timeZoneName: fromPattern.timeZoneName,
    era: fromPattern.era,
    year: fromPattern.year,
    month: fromPattern.month,
    day: fromPattern.day,
    weekday: fromPattern.weekday,
    hour12: fromPattern.hour12,
    hour: fromPattern.hour,
    minute: fromPattern.minute,
    second: fromPattern.second
  };
};


/**
 * Returns the subset of the given locale list for which this locale list
 * has a matching (possibly fallback) locale. Locales appear in the same
 * order in the returned list as in the input list.
 */
v8Intl.DateTimeFormat.supportedLocalesOf = function(locales, options) {
  return supportedLocalesOf('dateformat', locales, options);
};


/**
 * Returns a String value representing the result of calling ToNumber(date)
 * according to the effective locale and the formatting options of this
 * DateTimeFormat.
 */
function formatDate(formatter, dateValue) {
  native function NativeJSInternalDateFormat();

  var dateMs;
  if (dateValue === undefined) {
    dateMs = Date.now();
  } else {
    dateMs = Number(dateValue);
  }

  if (!isFinite(dateMs)) {
    throw new RangeException('Provided date is not in valid range.');
  }

  return NativeJSInternalDateFormat(formatter.formatter, new Date(dateMs));
};


addBoundMethod(v8Intl.DateTimeFormat, 'format', formatDate);


/**
 * Initializes the given object so it's a valid BreakIterator instance.
 * Useful for subclassing.
 */
function initializeBreakIterator(iterator, locales, options) {
  native function NativeJSCreateBreakIterator();

  if (options === undefined) {
    options = {};
  }

  var getOption = getGetOption(options, 'breakiterator');

  var internalOptions = {};

  internalOptions.type = getOption('type', 'string',
				   ['character', 'word', 'sentence', 'line'],
				   'word');

  var locale = resolveLocale('breakiterator', locales, options);

  var internalIterator = NativeJSCreateBreakIterator(locale.locale,
						     internalOptions);
  internalIterator.type = internalOptions.type;

  Object.defineProperty(iterator, 'iterator', {value: internalIterator});

  return iterator;
}


/**
 * Constructs v8Intl.BreakIterator object given optional locales and options
 * parameters.
 *
 * @constructor
 */
v8Intl.BreakIterator = function(locales, options) {
  if (!this || this === v8Intl) {
    // Constructor is called as a function.
    return new v8Intl.BreakIterator(locales, options);
  }

  return initializeBreakIterator(toObject(this), locales, options);
};


/**
 * BreakIterator resolvedOptions method.
 */
v8Intl.BreakIterator.prototype.resolvedOptions = function() {
  return {
    locale: this.iterator.locale,
    type: this.iterator.type
  };
};


/**
 * Returns the subset of the given locale list for which this locale list
 * has a matching (possibly fallback) locale. Locales appear in the same
 * order in the returned list as in the input list.
 */
v8Intl.BreakIterator.supportedLocalesOf = function(locales, options) {
  return supportedLocalesOf('breakiterator', locales, options);
};


/**
 * Adopts text to segment using the iterator. Old text, if present,
 * gets discarded.
 */
function adoptText(iterator, text) {
  native function NativeJSBreakIteratorAdoptText();
  NativeJSBreakIteratorAdoptText(iterator.iterator, String(text));
};


/**
 * Returns index of the first break in the string and moves current pointer.
 */
function first(iterator) {
  native function NativeJSBreakIteratorFirst();
  return NativeJSBreakIteratorFirst(iterator.iterator);
};


/**
 * Returns the index of the next break and moves the pointer.
 */
function next(iterator) {
  native function NativeJSBreakIteratorNext();
  return NativeJSBreakIteratorNext(iterator.iterator);
};


/**
 * Returns index of the current break.
 */
function current(iterator) {
  native function NativeJSBreakIteratorCurrent();
  return NativeJSBreakIteratorCurrent(iterator.iterator);
};


/**
 * Returns type of the current break.
 */
function breakType(iterator) {
  native function NativeJSBreakIteratorBreakType();
  return NativeJSBreakIteratorBreakType(iterator.iterator);
};


addBoundMethod(v8Intl.BreakIterator, 'adoptText', adoptText);
addBoundMethod(v8Intl.BreakIterator, 'first', first);
addBoundMethod(v8Intl.BreakIterator, 'next', next);
addBoundMethod(v8Intl.BreakIterator, 'current', current);
addBoundMethod(v8Intl.BreakIterator, 'breakType', breakType);


/**
 * Adds bound method to the prototype of the given object.
 */
function addBoundMethod(obj, methodName, implementation) {
  Object.defineProperty(obj.prototype, methodName, {
    get: function() {
	var internalName = '__bound' + methodName + '__';
        if (this[internalName] === undefined) {
          var that = this;
          var boundMethod = function(x, y) {
	      return implementation(that, x, y);
          }
          this[internalName] = boundMethod;
        }
        return this[internalName];
      },
    enumerable: false,
    configurable: true
  });
}


/**
 * Returns an intersection of locales and service supported locales.
 * Parameter locales is treated as a priority list.
 */
function supportedLocalesOf(service, locales, options) {
  if (service.match(SERVICE_RE) === null) {
    throw new Error('Internal error, wrong service type: ' + service);
  }

  // Provide defaults if matcher was not specified.
  if (options === undefined) {
    options = {};
  } else {
    options = toObject(options);
  }

  var getOption = getGetOption(options, service);
  var matcher = getOption(options, 'localeMatcher', 'string',
                          ['lookup', 'best fit'], 'best fit');

  var requestedLocales = initializeLocaleList(locales);

  // Cache these, they don't ever change per service.
  if (AVAILABLE_LOCALES[service] === undefined) {
    AVAILABLE_LOCALES[service] = NativeJSAvailableLocalesOf(service);
  }

  // Use either best fit or lookup algorithm to match locales.
  if (matcher === 'best fit') {
    return initializeLocaleList(bestFitSupportedLocalesOf(
        requestedLocales, AVAILABLE_LOCALES[service]));
  }

  return initializeLocaleList(lookupSupportedLocalesOf(
      requestedLocales, AVAILABLE_LOCALES[service]));
}


/**
 * Returns the subset of the provided BCP 47 language priority list for which
 * this service has a matching locale when using the BCP 47 Lookup algorithm.
 * Locales appear in the same order in the returned list as in the input list.
 */
function lookupSupportedLocalesOf(requestedLocales, availableLocales) {
  var matchedLocales = [];
  for (var i = 0; i < requestedLocales.length; ++i) {
    // Remove -u- extension.
    var locale = requestedLocales[i].replace(UNICODE_EXTENSION_RE, '');
    locale = renameUnsupportedLocale(locale);
    do {
      if (availableLocales[locale] !== undefined) {
        // Push requested locale not the resolved one.
        matchedLocales.push(requestedLocales[i]);
        break;
      }
      // Truncate locale if possible, if not break.
      var pos = locale.lastIndexOf('-');
      if (pos === -1) {
        break;
      }
      locale = locale.substring(0, pos);
    } while (true);
  }

  return matchedLocales;
}


/**
 * Returns the subset of the provided BCP 47 language priority list for which
 * this service has a matching locale when using the implementation
 * dependent algorithm.
 * Locales appear in the same order in the returned list as in the input list.
 */
function bestFitSupportedLocalesOf(requestedLocales, availableLocales) {
  // TODO(cira): implement better best fit algorithm.
  return lookupSupportedLocalesOf(requestedLocales, availableLocales);
}


/**
 * Returns a getOption function that extracts property value for given
 * options object. If property is missing it returns defaultValue. If value
 * is out of range for that property it throws RangeError.
 */
function getGetOption(options, caller) {
  if (options === undefined) {
    throw new Error('Internal ' + caller + ' error. ' +
                    'Default options are missing.');
  }

  function getOption(property, type, values, defaultValue) {
    if (options[property] !== undefined && options[property] !== null) {
      var value = options[property];
      switch (type) {
        case 'boolean':
          value = Boolean(value);
          break;
        case 'string':
          value = String(value);
          break;
        case 'number':
          value = Number(value);
          break;
        default:
          throw new Error('Internal error. Wrong value type.');
      }
      if (values !== undefined && values.indexOf(value) === -1) {
        throw new RangeError('Value ' + value + ' out of range for ' + caller +
                             ' options property ' + property);
      }

      return value;
    }

    return defaultValue;
  }

  return getOption;
}


/**
 * Compares a BCP 47 language priority list requestedLocales against the locales
 * in availableLocales and determines the best available language to meet the
 * request. Two algorithms are available to match the locales: the Lookup
 * algorithm described in RFC 4647 section 3.4, and an implementation dependent
 * best-fit algorithm. Independent of the locale matching algorithm, options
 * specified through Unicode locale extension sequences are negotiated
 * separately, taking the caller's relevant extension keys and locale data as
 * well as client-provided options into consideration. Returns an object with
 * a locale property whose value is the language tag of the selected locale,
 * and properties for each key in relevantExtensionKeys providing the selected
 * value for that key.
 */
function resolveLocale(service, requestedLocales, options) {
  requestedLocales = initializeLocaleList(requestedLocales);

  var getOption = getGetOption(options, service);
  var matcher = getOption('localeMatcher', 'string',
                          ['lookup', 'best fit'], 'best fit');
  var resolved;
  if (matcher === 'lookup') {
    resolved = lookupMatcher(service, requestedLocales);
  } else {
    resolved = bestFitMatcher(service, requestedLocales);
  }

  return resolved;
}


/**
 * Returns best matched supported locale and extension info using basic
 * lookup algorithm.
 */
function lookupMatcher(service, requestedLocales) {
  if (service.match(SERVICE_RE) === null) {
    throw new Error('Internal error, wrong service type: ' + service);
  }

  // Cache these, they don't ever change per service.
  if (AVAILABLE_LOCALES[service] === undefined) {
    AVAILABLE_LOCALES[service] = NativeJSAvailableLocalesOf(service);
  }

  for (var i = 0; i < requestedLocales.length; ++i) {
    // Remove all extensions.
    var locale = requestedLocales[i].replace(ANY_EXTENSION_RE, '');
    locale = renameUnsupportedLocale(locale);
    do {
      if (AVAILABLE_LOCALES[service][locale] !== undefined) {
        // Return the resolved locale and extension.
        var extensionMatch = requestedLocales[i].match(UNICODE_EXTENSION_RE);
        var extension = (extensionMatch === null) ? '' : extensionMatch[0];
        return {'locale': locale, 'extension': extension, 'position': i};
      }
      // Truncate locale if possible.
      var pos = locale.lastIndexOf('-');
      if (pos === -1) {
        break;
      }
      locale = locale.substring(0, pos);
    } while (true);
  }

  // Didn't find a match, return empty.
  return {'locale': '', 'extension': '', 'position': -1};
}


/**
 * Returns best matched supported locale and extension info using
 * implementation dependend algorithm.
 */
function bestFitMatcher(service, requestedLocales) {
  // TODO(cira): implement better best fit algorithm.
  return lookupMatcher(service, requestedLocales);
}


/**
 * Parses Unicode extension into key - value map.
 * Returns empty object if the extension string is invalid.
 * We are not concerned with the validity of the values at this point.
 */
function parseExtension(extension) {
  var extensionSplit = extension.split('-');

  // Assume ['', 'u', ...] input, but don't throw.
  if (extensionSplit.length <= 2 ||
      (extensionSplit[0] !== '' && extensionSplit[1] !== 'u')) {
    return {};
  }

  // Key is {2}alphanum, value is {3,8}alphanum.
  // Some keys may not have explicit values (booleans).
  var extensionMap = {};
  var previousKey = undefined;
  for (var i = 2; i < extensionSplit.length; ++i) {
    var length = extensionSplit[i].length;
    var element = extensionSplit[i];
    if (length === 2) {
      extensionMap[element] = undefined;
      previousKey = element;
    } else if (length >= 3 && length <=8 && previousKey !== undefined) {
      extensionMap[previousKey] = element;
      previousKey = undefined;
    } else {
      // There is a value that's too long, or that doesn't have a key.
      return {};
    }
  }

  return extensionMap;
}


/**
 * Converts unsupported locale into full form. For example, zh-TW becomes
 * zh-Hant-TW so it can be properly matched using the Lookup algorithm.
 */
function renameUnsupportedLocale(locale) {
  if (locale === 'zh-TW') return 'zh-Hant-TW';

  return locale;
}


/**
 * Converts parameter to an Object if possible.
 */
function toObject(value) {
  if (value === undefined || value === null) {
    throw new TypeError('Value cannot be converted to an Object.');
  }

  return Object(value);
}


/**
 * Populates internalOptions object with boolean key-value pairs
 * from extensionMap and options.
 * Returns filtered extension (number and date format constructors use
 * Unicode extensions for passing parameters to ICU).
 * It's used for extension-option pairs only, e.g. kn-normalization, but not
 * for 'sensitivity' since it doesn't have extension equivalent.
 * Extensions like nu and ca don't have options equivalent, so we place
 * undefined in the map.property to denote that.
 */
function setOptions(inOptions, extensionMap, keyValues, getOption, outOptions) {
  var extension = '';

  function updateExtension(key, value) {
    return '-' + key + '-' + String(value);
  }

  function updateProperty(property, type, value) {
    if (type === 'boolean' && (typeof value === 'string')) {
      value = (value === 'true') ? true : false;
    }

    if (property !== undefined) {
      outOptions[property] = value;
    }
  }

  for (var key in keyValues) {
    if (keyValues.hasOwnProperty(key)) {
      var value = undefined;
      var map = keyValues[key];
      if (map.property !== undefined) {
        // This may return true if user specifies numeric: 'false', since
        // Boolean('nonempty') === true.
        value = getOption(map.property, map.type, map.values);
      }
      if (value !== undefined) {
        updateProperty(map.property, map.type, value);
        extension += updateExtension(key, value);
        continue;
      }
      // User options didn't have it, check Unicode extension.
      // Here we want to convert strings 'true', 'false' into proper Boolean
      // values (not a user error).
      if (extensionMap.hasOwnProperty(key)) {
        value = extensionMap[key];
        if (value !== undefined) {
          updateProperty(map.property, map.type, value);
          extension += updateExtension(key, value);
        } else if (map.type === 'boolean') {
          // Boolean keys are allowed not to have values in Unicode extension.
          // Those default to true.
          updateProperty(map.property, map.type, true);
          extension += updateExtension(key, true);
        }
      }
    }
  }

  return extension === ''? '' : '-u' + extension;
}


// Fix RegExp global state so we don't fail WebKit layout test:
// fast/js/regexp-caching.html
// It seems that 'g' or test() operations leave state changed.
var CLEANUP_RE = new RegExp('');
CLEANUP_RE.test('');

return v8Intl;
}());
