// Copyright 2012 the v8-i18n authors.
//
// Licensed under the Apache License, Version 2.0 (the 'License');
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an 'AS IS' BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Some locales should be mapped into full form, like zh-TW into zh-Hant-TW,
// when used with lookup algorithm.

var nf = Intl.NumberFormat(['zh-TW'], {localeMatcher: 'lookup'});
assertEquals('zh-Hant-TW', nf.resolvedOptions().locale);

// Don't map zh-CN, zh already represents it.
nf = Intl.NumberFormat(['zh-CN'], {localeMatcher: 'lookup'});
assertEquals('zh', nf.resolvedOptions().locale);

// Make sure en-US still maps to proper locale.
nf = Intl.NumberFormat(['en-US'], {localeMatcher: 'lookup'});
assertEquals('en-US', nf.resolvedOptions().locale);
