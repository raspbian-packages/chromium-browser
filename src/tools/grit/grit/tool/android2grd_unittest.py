#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''Unit tests for grit.tool.android2grd'''

import os
import sys
if __name__ == '__main__':
  sys.path[0] = os.path.abspath(os.path.join(sys.path[0], '../..'))

import unittest
import xml.dom.minidom

from grit.tool import android2grd


class Android2GrdUnittest(unittest.TestCase):

  def __Parse(self, xml_string):
    return xml.dom.minidom.parseString(xml_string).childNodes[0]

  def testCreateTclibMessage(self):
    tool = android2grd.Android2Grd()
    msg = tool.CreateTclibMessage(self.__Parse(r'''
        <string name="simple">A simple string</string>'''))
    self.assertEqual(msg.GetRealContent(), 'A simple string')
    msg = tool.CreateTclibMessage(self.__Parse(r'''
        <string name="outer_whitespace">
          Strip leading/trailing whitespace
        </string>'''))
    self.assertEqual(msg.GetRealContent(), 'Strip leading/trailing whitespace')
    msg = tool.CreateTclibMessage(self.__Parse(r'''
        <string name="inner_whitespace">Fold  multiple   spaces</string>'''))
    self.assertEqual(msg.GetRealContent(), 'Fold multiple spaces')
    msg = tool.CreateTclibMessage(self.__Parse(r'''
        <string name="escaped_spaces">Retain \n escaped\t spaces</string>'''))
    self.assertEqual(msg.GetRealContent(), 'Retain \n escaped\t spaces')
    msg = tool.CreateTclibMessage(self.__Parse(r'''
        <string name="quotes">   " Quotes  preserve
          whitespace"  but  only  for  "enclosed   elements  "
        </string>'''))
    self.assertEqual(msg.GetRealContent(), ''' Quotes  preserve
          whitespace but only for enclosed   elements  ''')
    msg = tool.CreateTclibMessage(self.__Parse(
        r'''<string name="escaped_characters">Escaped characters: \"\'\\\t\n'''
        '</string>'))
    self.assertEqual(msg.GetRealContent(), '''Escaped characters: "'\\\t\n''')
    msg = tool.CreateTclibMessage(self.__Parse(
        '<string xmlns:xliff="urn:oasis:names:tc:xliff:document:1.2" '
        'name="placeholders">'
        'Open <xliff:g id="FILENAME" example="internet.html">%s</xliff:g>?'
        '</string>'))
    self.assertEqual(msg.GetRealContent(), 'Open %s?')
    self.assertEqual(len(msg.GetPlaceholders()), 1)
    self.assertEqual(msg.GetPlaceholders()[0].presentation, 'FILENAME')
    self.assertEqual(msg.GetPlaceholders()[0].original, '%s')
    self.assertEqual(msg.GetPlaceholders()[0].example, 'internet.html')
    msg = tool.CreateTclibMessage(self.__Parse(r'''
        <string name="comment">Contains a <!-- ignore this --> comment
        </string>'''))
    self.assertEqual(msg.GetRealContent(), 'Contains a comment')

  def testIsTranslatable(self):
    tool = android2grd.Android2Grd()
    string_el = self.__Parse('<string>Hi</string>')
    self.assertTrue(tool.IsTranslatable(string_el))
    string_el = self.__Parse(
        '<string translatable="true">Hi</string>')
    self.assertTrue(tool.IsTranslatable(string_el))
    string_el = self.__Parse(
        '<string translatable="false">Hi</string>')
    self.assertFalse(tool.IsTranslatable(string_el))

if __name__ == '__main__':
  unittest.main()

