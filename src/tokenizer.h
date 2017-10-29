#ifndef SRC_TOKENIZER_H
#define SRC_TOKENIZER_H

#include <regex>
#include <string>
#include <vector>

#include "sql_errors.h"

class Tokenizer {
 public:
  Tokenizer();

  void CleanupSpaces(std::string& input);

  bool ReadOneWord(const std::string list, std::string& word);
};

#endif // SRC_TOKENIZER_H
