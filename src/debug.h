#ifndef SRC_DEBUG_H
#define SRC_DEBUG_H

#include <iostream>

#if defined(DEBUG)
#define DEBUG_MSG(str) do { std::cout << __FILE__ << ": " << __FUNCTION__ << \
    ": " << __LINE__ << ":\t " << str << std::endl; } while( false )
#else
#define DEBUG_MSG(str) do { } while ( false )
#endif

#endif // SRC_DEBUG_H
