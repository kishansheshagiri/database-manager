#ifndef SRC_TOKENIZER_H
#define SRC_TOKENIZER_H

#include <string>
#include <vector>

#include "sql_errors.h"

class Tokenizer {
 public:
  Tokenizer(std::string input_string);

  void Tokenize(SqlErrors::Type &error_code);
  int WordCount(SqlErrors::Type &error_code);
  std::string Word(int index, SqlErrors::Type &error_code);
 private:
  void cleanupOperators();

  bool tokenized_;
  std::string input_string_;
  std::vector<std::string> token_list_;
  std::string characters_to_cleanup_;
};

#endif // SRC_TOKENIZER_H
