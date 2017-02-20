/*********************************************************************
Copyright 2013  JST ERATO Minato project and other contributors
http://www-erato.ist.hokudai.ac.jp/?language=en

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**********************************************************************/

#include "graphillion/util.h"

#include "graphillion/type.h"

#ifdef WIN32
#include "../mingw32/strtok_r.hpp"
#else
#ifdef _WIN32
#define strtok_r strtok_s
#endif
#endif

namespace graphillion {

using std::string;
using std::vector;

vector<string> split(const string& str, const string sep) {
  vector<char> buf;
  for (string::const_iterator c = str.begin(); c != str.end(); ++c)
    buf.push_back(*c);
  buf.push_back('\0');
  vector<string> v;
  char* last;
  char* p = strtok_r(buf.data(), sep.c_str(), &last);
  while (p != NULL) {
    v.push_back(p);
    p = strtok_r(NULL, sep.c_str(), &last);
  }
  return v;
}

}  // namespace graphillion
