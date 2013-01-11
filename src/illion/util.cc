#include "illion/util.h"

namespace illion {

std::vector<std::string> split(const std::string& str,
                               const std::string sep) {
  std::vector<char> buf;
  for (const auto& c : str)
    buf.push_back(c);
  buf.push_back('\0');
  std::vector<std::string> v;
  char* last;
  char* p = strtok_r(buf.data(), sep.c_str(), &last);
  while (p != nullptr) {
    v.push_back(p);
    p = strtok_r(nullptr, sep.c_str(), &last);
  }
  return v;
}

}  // namespace illion
