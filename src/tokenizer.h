#ifndef SRC_TOKENIZER_H
#define SRC_TOKENIZER_H

#include <regex>
#include <string>
#include <vector>

#include "sql_errors.h"

class Tokenizer {
 public:
  Tokenizer(std::string input_string);

  void Tokenize(SqlErrors::Type &error_code);
  void HandleSelectAll();

  int WordCount(SqlErrors::Type &error_code);
  std::string Word(int index, SqlErrors::Type &error_code);

  static void SplitIntoWords(
      const std::string &long_string, const std::string &delimiter,
      std::vector<std::string> &result);
  static void SplitIntoWordsMultiDelimiters(
      const std::string &long_string, const std::string &delimiter,
      std::vector<std::string> &result);
 private:
  void cleanupOperators();
  static void splitIntoWords(
      const std::string &long_string, const std::regex& split_regex,
      std::vector<std::string> &result);

  bool tokenized_;
  std::string input_string_;
  std::vector<std::string> token_list_;
  std::string characters_to_cleanup_;
};

#endif // SRC_TOKENIZER_H
