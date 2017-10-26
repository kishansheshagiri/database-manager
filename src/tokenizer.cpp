#include "tokenizer.h"

#include <regex>
#include <sstream>

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

  cleanupOperators();

  std::istringstream stream_tokenizer(input_string_);
  std::string token;

  while(std::getline(stream_tokenizer, token, ' '))
    token_list_.push_back(token);

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
    std::regex replace_space(" *[\\" + char_as_string + "]+ *");
    input_string_ = std::regex_replace(input_string_, replace_space, char_as_string);
  }
}
