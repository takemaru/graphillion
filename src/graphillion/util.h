#ifndef GRAPHILLION_UTIL_H_
#define GRAPHILLION_UTIL_H_

#include <cstring>

#include <sstream>
#include <string>
#include <vector>

namespace graphillion {

#ifdef __PRETTY_FUNCTION__
#define __FUNC__ (__PRETTY_FUNCTION__)
#else
#define __FUNC__ (__func__)
#endif

#ifndef assert
#define assert(e) {                                                     \
    if (!(e)) {                                                          \
      fprintf(stderr, "Error: %s:%u: %s: assertion `%s' failed.\n",     \
              __FILE__, __LINE__, __FUNC__, (#e));                      \
      exit(1);                                                          \
    }                                                                   \
  }
#endif

#define error_if(e, m) {                                                \
    if ((e)) {                                                          \
      std::stringstream ss;                                             \
      (ss << m);                                                        \
      fprintf(stderr, "Error: %s:%u: %s: %s, assertion `%s' failed.\n", \
              __FILE__, __LINE__, __FUNC__, ss.str().c_str(), (#e));    \
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
                               const std::string sep = " ");

inline bool is_space(std::string s) {
  return s.find_first_not_of(" \t\r\n") == std::string::npos;
}

inline bool is_digit(std::string s) {
  return s.find_first_not_of("0123456789") == std::string::npos;
}

}  // namespace graphillion

#endif  // GRAPHILLION_UTIL_H_
