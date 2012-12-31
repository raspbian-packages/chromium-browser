// Flags: --expose-i18n

var LOCALES = [
  'af', 'am', 'ar', 'bg', 'bn', 'ca', 'cs', 'da', 'de', 'el', 'en', 'en-US',
  'en-GB', 'es', 'es-419', 'es-ES', 'et', 'eu', 'fa', 'fi', 'fil', 'fr',
  'fr-CA', 'fr-FR', 'gl', 'gu', 'he', 'hi', 'hr', 'hu', 'id', 'is', 'it', 'iw',
  'ja', 'kn', 'ko', 'lt', 'lv', 'nl', 'ml', 'mr', 'ms', 'nb', 'no', 'pl', 'pt',
  'pt-BR', 'pt-PT', 'ro', 'ru', 'sk', 'sl', 'sr', 'sv', 'sw', 'ta', 'te', 'th',
  'tr', 'uk', 'ur', 'vi', 'zh', 'zh-Hans-CN', 'zh-Hant-HK', 'zh-Hant-TW',
  'zh-CN', 'zh-HK', 'zh-TW', 'zu',
];

var BREAK_TYPES = ['unknown', 'none', 'number', 'word', 'kana', 'ideo'];

var TEST_STRINGS = [
  'The answer is 42.',
  '色は匂へど散りぬるを我が世誰ぞ常ならむ有為の奥山今日越えて浅き夢見じ酔ひもせず（ん）',
  '',
  'GOOG411',
  'The answer is still 42.'
];

var reverseBreakTypes;
var locale, breaker;
var typeName, testString;
var i, j, k;
var nextIndex, nextBreakType;
var key, value;

// Test if the break type constants exist and are numbers
for (j = 0; j < BREAK_TYPES.length; j += 1) {
  typeName = BREAK_TYPES[j];
  assertEquals('number', typeof v8Locale.v8BreakIterator.BreakType[typeName]);
}

// Fill reverseBreakTypes
reverseBreakTypes = {};
for (key in v8Locale.v8BreakIterator.BreakType) {
  value = v8Locale.v8BreakIterator.BreakType[key];
  reverseBreakTypes[value] = key;
}

for (i = 0; i < LOCALES.length; i += 1) {
  locale = new v8Locale(LOCALES[i]);
  
  // Test if v8CreateBreakIterator exists
  assertInstanceof(locale.v8CreateBreakIterator, Function);
  
  // Test if methods could be added to locale
  locale.createIterator = function() {
    return this.v8CreateBreakIterator('word');
  };
  
  // Create an iterator through the convenience method
  breaker = locale.createIterator();
  
  // Test if certain methods exists
  assertInstanceof(breaker.adoptText, Function);
  assertInstanceof(breaker.next, Function);
  assertInstanceof(breaker.breakType, Function);
  
  for (j = 0; j < TEST_STRINGS.length; j += 1) {
    testString = TEST_STRINGS[j];
    breaker.adoptText(testString);
    do {
      // Test if breakType returns valid values before calling next()
      nextBreakType = breaker.breakType();
      assertEquals('string', typeof reverseBreakTypes[nextBreakType]);

      nextIndex = breaker.next();
      // Test if return value of next() is in range
      assertTrue(-1 <= nextIndex && nextIndex <= testString.length);

      // Make sure breakType returns valid values after calling next()
      nextBreakType = breaker.breakType();
      assertEquals('string', typeof reverseBreakTypes[nextBreakType]);
    } while (nextIndex !== -1);
  }
}

