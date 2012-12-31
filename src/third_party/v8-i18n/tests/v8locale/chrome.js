// Flags: --expose-i18n

// Test v8Locale, as presently used in Chrome file manager
// at: src/chrome/browser/resources/file_manager/js/file_manager.js

// Test proper locale creation
var testLocaleCreation = function(localeCode) {
  var NORMALIZATIONS = {
    'zh-CN': 'zh',
    'zh-TW': 'zh'
    // TODO(cira): The real expected values are these:
    //'zh-CN': 'zh-Hans-CN',
    //'zh-TW': 'zh-Hant-TW',
  };

  var locale = new v8Locale(localeCode);
  var expectedLocaleCode = NORMALIZATIONS[localeCode] || localeCode;

  assertInstanceof(locale, v8Locale);
  assertEquals(expectedLocaleCode, locale.options.localeID);

  return locale;
}


// Test date formatter
var testDateFormatter = function(locale, options) {
  var DATES = [
    new Date(2012, 2, 20, 5, 14, 27),
    new Date(1970, 0, 1, 0, 0, 0),
    new Date(1968, 11, 31, 23, 59, 59)
  ];

  var formatter, date;
  var i;

  formatter = locale.createDateTimeFormat(options);
  assertInstanceof(formatter.format, Function);

  for (i = 0; i < DATES.length; i += 1) {
    date = DATES[i];
    formatted_date = formatter.format(date);
    assertEquals('string', typeof formatted_date);
    assertTrue(formatted_date !== '');
  }
}


// Test collator
var testCollator = function(locale) {
  var STRINGS = [
    '', 'alpha', 'Bravo', 'Alpha', 'cafe', 'café', 'Café', 'fi', 'f\u200Ci',
    'zebra', 'aaland', 'Zebra', '\u592B', '\u4E87', 'm\u030C', 'm\u0300', 'Z'
  ];

  var collator, comparison_result;
  var i, j;

  collator = locale.createCollator({
    'numeric': true, 'ignoreCase': true, 'ignoreAccents': true
  });

  for (i = 0; i < STRINGS.length; i += 1) {
    for (j = 0; j < STRINGS.length; j += 1) {
      comparison_result = collator.compare(STRINGS[i], STRINGS[j]);
      if (i == j) {
        assertEquals(0, comparison_result);
      } else {
        assertTrue(comparison_result == 0 ||
                   comparison_result == 1 ||
                   comparison_result == -1);
      }
    }
  }
}


// Main test function
var test = function() {
  // There are Chrome localizations with 'iw' and 'no' too,
  // but Chrome calls us with 'he' and 'nb' in those locales.
  var CHROME_LOCALE_CODES = [
    'am', 'ar', 'bg', 'bn', 'ca', 'cs', 'da', 'de', 'el', 'en-US', 'en-GB',
    'es', 'es-419', 'et', 'fa', 'fi', 'fil', 'fr', 'gu', 'he', 'hi', 'hr', 'hu',
    'id', 'it', 'ja', 'kn', 'ko', 'lt', 'lv', 'ml', 'mr', 'nb', 'nl', 'pl',
    'pt-BR', 'pt-PT', 'ro', 'ru', 'sk', 'sl', 'sr', 'sv', 'sw', 'ta', 'te',
    'th', 'tr', 'uk', 'vi', 'zh-CN', 'zh-TW'
  ];

  var SKELETONS = ['hh:mm:ss', 'mm:ss'];

  var locale, locale_code;
  var i, j;

  for (i = 0; i < CHROME_LOCALE_CODES.length; i += 1) {
    locale_code = CHROME_LOCALE_CODES[i];

    locale = testLocaleCreation(CHROME_LOCALE_CODES[i]);

    // Test date formatter with dateType
    testDateFormatter(locale, {'dateType': 'medium'});

    // Test date formatter with skeletons
    for (j = 0; j < SKELETONS.length; j += 1) {
      testDateFormatter(locale, {skeleton: SKELETONS[j]});
    }

    testCollator(locale);
  }
}


test();
