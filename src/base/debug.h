#ifndef SRC_DEBUG_H
#define SRC_DEBUG_H

#include <iostream>

#define ERROR_MSG(str) do { std::cout << str << std::endl; } while (false)

#if defined(DEBUG)
#define DEBUG_MSG(str) do { std::cout << __FILE__ << ": " << __FUNCTION__ << \
    ": " << __LINE__ << ":\t " << str << std::endl; } while (false)
#else
#define DEBUG_MSG(str) do { } while (false)
#endif

#if defined(LOGGING)
#define LOG_MSG(str) DEBUG_MSG(str)
#else
#define LOG_MSG(str) do { } while (false)
#endif

#endif // SRC_DEBUG_H
