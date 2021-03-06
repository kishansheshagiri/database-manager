#ifndef SRC_BASE_TOKENIZER_H
#define SRC_BASE_TOKENIZER_H

#include <string>

class Tokenizer {
 public:
  Tokenizer();

  static bool ReadOneWord(const std::string& list, std::string& word,
      char& separator, int& leading_spaces);
  static bool ReadLiteral(const std::string& input, std::string& literal);
  static bool SplitIntoTwo(const std::string& word, const char& separator,
      std::string& first, std::string& second);
};

#endif // SRC_BASE_TOKENIZER_H
