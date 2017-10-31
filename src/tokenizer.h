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
      char& separator, int& leading_spaces);
  bool ReadLiteral(const std::string& input, std::string& literal);
};

#endif // SRC_TOKENIZER_H
