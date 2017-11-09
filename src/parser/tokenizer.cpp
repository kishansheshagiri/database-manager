#include "tokenizer.h"

#include "base/debug.h"
#include "base/sql_errors.h"

const std::string symbol_list = " ,.()<>=+-*";

Tokenizer::Tokenizer() {
}

bool Tokenizer::ReadOneWord(const std::string& list, std::string& word,
    char& separator, int& leading_spaces) {
  // Intialize so that we never pass the wrong values.
  word.clear();
  separator = '\0';
  leading_spaces = 0;

  std::size_t symbol_position = list.find_first_of(symbol_list);
  if (symbol_position == std::string::npos) {
    word = list;
    return true;
  }

  separator = list[symbol_position];
  if (symbol_position == 0) {
    word = list[symbol_position];
  } else {
    word = list.substr(0, symbol_position);
  }

  if (separator == ' ') {
    leading_spaces = 1;
  }

  while (symbol_position < (list.size() - 1) &&
      list[++symbol_position] == ' ') {
    leading_spaces++;
  }

  LOG_MSG("Word: " << word << ", separator: " << separator\
      << ", leading_spaces: " << leading_spaces);
  return true;
}

bool Tokenizer::ReadLiteral(const std::string& input, std::string& literal) {
  literal.clear();
  if (input[0] == '\"') {
    int it = 1;
    while (input[it] != '\"' && it < input.length()) {
      literal.push_back(input[it++]);
    }

    LOG_MSG("Literal: " << literal);
    return true;
  }

  LOG_MSG("");
  return false;
}