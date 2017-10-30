#ifndef SRC_TOKENIZER_H
#define SRC_TOKENIZER_H

#include <regex>
#include <string>
#include <vector>

#include "sql_errors.h"

class Tokenizer {
 public:
  Tokenizer();

  bool ReadOneWord(const std::string& list, std::string& word,
      std::string& trailing_characters);
  bool ReadOneWord(const std::string& list, std::string& word);
};

#endif // SRC_TOKENIZER_H
