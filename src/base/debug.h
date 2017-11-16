#ifndef SRC_DEBUG_H
#define SRC_DEBUG_H

#include <iostream>

#define ERROR_MSG(str) do { \
  std::cout.clear(); \
  std::cout << str << std::endl; \
  std::cout.setstate(std::ios_base::failbit); \
} while (false)

#if defined(DEBUG)
#define DEBUG_MSG(str) do { \
  std::cout.clear(); \
  std::cout << __FILE__ << ": " << __FUNCTION__ << \
      ": " << __LINE__ << ":\t " << str << std::endl; \
  std::cout.setstate(std::ios_base::failbit); \
} while (false)
#else
#define DEBUG_MSG(str) do { } while (false)
#endif

#if defined(LOGGING)
#define LOG_MSG(str) DEBUG_MSG(str)
#else
#define LOG_MSG(str) do { } while (false)
#endif

namespace logging {

void inline DisableConsoleLogs() {
#if !defined(DEBUG) && !defined(LOGGING)
  std::cout.setstate(std::ios_base::failbit);
  std::cerr.setstate(std::ios_base::failbit);
#endif
}

}

#endif // SRC_DEBUG_H
