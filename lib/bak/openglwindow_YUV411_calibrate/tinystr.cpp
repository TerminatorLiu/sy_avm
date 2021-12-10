/******************************************************************************
 * Copyright 2017 The SANY Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#ifndef TIXML_USE_STL

//#include "modules/surroundview/openglwindow/tinystr.h"
#include "tinystr.h"

// Error value for find primitive
const TiXmlString::size_type TiXmlString::npos =
    static_cast<TiXmlString::size_type>(-1);

// Null rep.
TiXmlString::Rep TiXmlString::nullrep_ = {0, 0, {'\0'}};

void TiXmlString::reserve(size_type cap) {
  if (cap > capacity()) {
    TiXmlString tmp;
    tmp.init(length(), cap);
    memcpy(tmp.start(), data(), length());
    swap(tmp);
  }
}

TiXmlString &TiXmlString::assign(const char *str, size_type len) {
  size_type cap = capacity();
  if (len > cap || cap > 3 * (len + 8)) {
    TiXmlString tmp;
    tmp.init(len);
    memcpy(tmp.start(), str, len);
    swap(tmp);
  } else {
    memmove(start(), str, len);
    set_size(len);
  }
  return *this;
}

TiXmlString &TiXmlString::append(const char *str, size_type len) {
  size_type newsize = length() + len;
  if (newsize > capacity()) {
    reserve(newsize + capacity());
  }
  memmove(finish(), str, len);
  set_size(newsize);
  return *this;
}

TiXmlString operator+(const TiXmlString &a, const TiXmlString &b) {
  TiXmlString tmp;
  tmp.reserve(a.length() + b.length());
  tmp += a;
  tmp += b;
  return tmp;
}

TiXmlString operator+(const TiXmlString &a, const char *b) {
  TiXmlString tmp;
  TiXmlString::size_type b_len = static_cast<TiXmlString::size_type>(strlen(b));
  tmp.reserve(a.length() + b_len);
  tmp += a;
  tmp.append(b, b_len);
  return tmp;
}

TiXmlString operator+(const char *a, const TiXmlString &b) {
  TiXmlString tmp;
  TiXmlString::size_type a_len = static_cast<TiXmlString::size_type>(strlen(a));
  tmp.reserve(a_len + b.length());
  tmp.append(a, a_len);
  tmp += b;
  return tmp;
}

#endif  // TIXML_USE_STL
