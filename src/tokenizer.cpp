#include "tokenizer.h"

#include <regex>

#include "debug.h"

Tokenizer::Tokenizer(std::string input_string)
  : input_string_(input_string),
    characters_to_cleanup_(",<>=+-*"),
    tokenized_(false) {
}

void Tokenizer::Tokenize(SqlErrors::Type &error_code) {
  if (input_string_.length() == 0) {
    error_code = SqlErrors::TOKENIZER_EMPTY_INPUT;
    return;
  }

  #ifdef DEBUG
    std::cout << "Input: " << input_string_ << std::endl;
  #endif

  cleanupOperators();

  std::regex re_tokenize(
      "\\s+(?=(?:[^\\\"]*\\\"[^\\\"]*\\\"[^\\\"]*|[^\\\"])*$)");
  std::sregex_token_iterator
      first{input_string_.begin(), input_string_.end(), re_tokenize, -1},
      last;
  token_list_ = {first, last};

  #ifdef DEBUG
    std::cout << "Tokens:" << std::endl;
    for (auto printer:token_list_) std::cout << "\t" << printer << std::endl;
  #endif

  tokenized_ = true;
}

int Tokenizer::WordCount(SqlErrors::Type &error_code) {
  if (tokenized_)
    return token_list_.size();

  error_code = SqlErrors::TOKENIZER_NOT_INITIALIZED;
  return -1;
}

std::string Tokenizer::Word(int index, SqlErrors::Type &error_code) {
  if (!tokenized_) {
    error_code = SqlErrors::TOKENIZER_NOT_INITIALIZED;
    return nullptr;
  }

  if (index < token_list_.size())
    return token_list_[index];

  error_code = SqlErrors::TOKENIZER_INVALID_INDEX;
  return nullptr;
}

void Tokenizer::cleanupOperators() {
  for (auto &operator_char : characters_to_cleanup_) {
    std::string char_as_string = std::string(1, operator_char);
    std::regex replace_space(" *[\\" + char_as_string +
        "]+ *(?=(?:[^\\\"]*\\\"[^\\\"]*\\\"[^\\\"]*|[^\\\"])*$)");
    input_string_ = std::regex_replace(input_string_,
        replace_space,char_as_string);
  }
}
