#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Produces localized strings.xml files for Android.

In cases where an "android" type output file is requested in a grd, the classes
in android_xml will process the messages and translations to produce a valid
strings.xml that is properly localized with the specified language.

For example if the following output tag were to be included in a grd file
<outputs>
  ...
  <output filename="values-es/strings.xml" type="android" lang="es" />
  ...
</outputs>

for a grd file with the following messages:

  <message name="IDS_HELLO" desc="Simple greeting">Hello</message>
  <message name="IDS_WORLD" desc="The world">world</message>

and there existed an appropriate xtb file containing the Spanish translations,
then the output would be:

<?xml version="1.0" encoding="utf-8"?>
<resources xmlns:android="http://schemas.android.com/apk/res/android">
  <string name="hello">"Hola"</string>
  <string name="world">"mundo"</string>
</resources>

which would be written to values-es/strings.xml and usable by the Android
resource framework.
"""

import types
import xml.sax.saxutils

from grit import lazy_re
from grit.format import interface
from grit.node import message


class ResourcesElement(interface.ItemFormatter):
  """Surrounds the messages with the required begin and end blocks."""

  def Format(self, item, lang='en', output_dir='.'):
    assert isinstance(lang, types.StringTypes)
    return ('<?xml version="1.0" encoding="utf-8"?>\n'
            '<resources '
            'xmlns:android="http://schemas.android.com/apk/res/android">\n')

  def FormatEnd(self, item, lang='en', output_dir='.'):
    assert isinstance(lang, types.StringTypes)
    return '</resources>\n'


class Message(interface.ItemFormatter):
  """Writes out a single string as a <resource/> element."""

  # The Android resource name and optional product information are placed
  # in the grd string name because grd doesn't know about Android product
  # information.
  _NAME_PATTERN = lazy_re.compile(
      'IDS_(?P<name>[A-Z0-9_]+)(_product_(?P<product>[a-z]+))?\Z')

  # In most cases we only need a name attribute and string value.
  _SIMPLE_TEMPLATE = u'<string name="%s">%s</string>\n'

  # In a few cases a product attribute is needed.
  _PRODUCT_TEMPLATE = u'<string name="%s" product="%s">%s</string>\n'

  def Format(self, item, lang='en', output_dir='.'):
    assert isinstance(lang, types.StringTypes)
    assert isinstance(item, message.MessageNode)

    value = item.ws_at_start + item.Translate(lang) + item.ws_at_end
    # Replace < > & with &lt; &gt; &amp; to ensure we generate valid XML and
    # replace ' " with \' \" to conform to Android's string formatting rules.
    value = xml.sax.saxutils.escape(value, {"'": "\\'", '"': '\\"'})
    # Wrap the string in double quotes to preserve whitespace.
    value = '"' + value + '"'

    mangled_name = item.GetTextualIds()[0]
    match = Message._NAME_PATTERN.match(mangled_name)
    if not match:
      raise Exception('Unexpected resource name: %s' % mangled_name)
    name = match.group('name').lower()
    product = match.group('product')
    if product:
      return Message._PRODUCT_TEMPLATE % (name, product, value)
    else:
      return Message._SIMPLE_TEMPLATE % (name, value)
