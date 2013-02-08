#include "graphillion/util.h"

#include "graphillion/type.h"

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
  while (p != nullptr) {
    v.push_back(p);
    p = strtok_r(nullptr, sep.c_str(), &last);
  }
  return v;
}

}  // namespace graphillion
