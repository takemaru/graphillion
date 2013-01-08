#ifndef ILLION_UTIL_H_
#define ILLION_UTIL_H_

#include <cstring>

#include <sstream>
#include <string>
#include <vector>

namespace illion {

#define error_if(e, m) {                                                \
    if ((e)) {                                                          \
      std::stringstream ss;                                             \
      (ss << m);                                                        \
      fprintf(stderr, "Error: %s:%u: %s: %s, assertion `%s' failed.\n", \
              __FILE__, __LINE__, __PRETTY_FUNCTION__, ss.str().c_str(), (#e)); \
      exit(1);                                                          \
    }                                                                   \
  }

template <class T>
std::string join(const std::vector<T>& v, std::string sep = " ") {
  std::stringstream ss;
  for (int i = 0; i < static_cast<int>(v.size()); ++i) {
    ss << v[i];
    if (i < static_cast<int>(v.size()) - 1) ss << sep;
  }
  return ss.str();
}

std::vector<std::string> split(const std::string& str,
                               const std::string sep = " ") {
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

inline bool is_space(std::string s) {
  return s.find_first_not_of(" \t\r\n") == std::string::npos;
}

inline bool is_digit(std::string s) {
  return s.find_first_not_of("0123456789") == std::string::npos;
}

}  // namespace illion

#endif  // ILLION_UTIL_H_
