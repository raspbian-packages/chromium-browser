# Copyright 2011 the v8-i18n authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

{
  'variables': {
    # Use build/gyp_chromium -D"icu_path=path" to override paths.
    # gyp_chromium script can be found in chromium project.
    # TODO(cira): We may need to add another ICU variable for include path
    # since chromium version of ICU differs from original ICU checkout.
    'icu_path%': '../../icu',
    'v8_path%': '../../..',
  },
  'targets': [
    {
      'target_name': 'v8-i18n',
      'type': 'static_library',
      'sources': [
        '../include/extension.h',
        '../src/break-iterator.cc',
        '../src/break-iterator.h',
        '../src/collator.cc',
        '../src/collator.h',
        '../src/datetime-format.cc',
        '../src/datetime-format.h',
        '../src/extension.cc',
	'../src/intl-break-iterator.cc',
	'../src/intl-break-iterator.h',
        '../src/intl-collator.cc',
        '../src/intl-collator.h',
        '../src/intl-date-format.cc',
        '../src/intl-date-format.h',
        '../src/intl-number-format.cc',
        '../src/intl-number-format.h',
        '../src/language-matcher.cc',
        '../src/language-matcher.h',
        '../src/locale-list.cc',
        '../src/locale-list.h',
        '../src/locale.cc',
        '../src/locale.h',
        '../src/natives.h',
        '../src/number-format.cc',
        '../src/number-format.h',
        '../src/utils.cc',
        '../src/utils.h',
        '<(SHARED_INTERMEDIATE_DIR)/v8-i18n-js.cc',
      ],
      'include_dirs': [
        '..',
        '<(v8_path)',
      ],
      'dependencies': [
        '<(icu_path)/icu.gyp:*',
        '<(v8_path)/v8/tools/gyp/v8.gyp:v8',
        'api2c#host',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../..',
        ],
      },
    },
    {
      'target_name': 'api2c',
      'type': 'none',
      'toolsets': ['host'],
      'variables': {
        'js_files': [
          '../src/api.js',
          '../src/intl.js'
        ],
      },
      'actions': [
        {
          'action_name': 'api2c',
          'inputs': [
            '../tools/js2c.py',
            '<@(js_files)',
          ],
          'outputs': [
            '<(SHARED_INTERMEDIATE_DIR)/v8-i18n-js.cc',
          ],
          'action': [
            'python',
            '../tools/js2c.py',
            '<@(_outputs)',
            '<@(js_files)'
          ],
        },
      ],
    },
  ],  # targets
}
