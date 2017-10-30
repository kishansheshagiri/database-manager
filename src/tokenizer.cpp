#include "tokenizer.h"

#include "debug.h"
#include "sql_errors.h"

const std::string symbol_list = ", .()<>=+-*";

Tokenizer::Tokenizer() {
}

bool Tokenizer::ReadOneWord(const std::string& list, std::string& word,
    std::string& trailing_characters) {
  std::size_t symbol_position = list.find_first_of(symbol_list);
  if (symbol_position == std::string::npos) {
    DEBUG_MSG("No separator found");
    return false;
  }

  if (symbol_position == 0 && list[0] != ' ') {
    DEBUG_MSG("Found leading symbol");
    trailing_characters.push_back(list[0]);
    return false;
  }

  word = list.substr(0, symbol_position);
  std::string word_end = list.substr(symbol_position);
  while (word_end.find_first_of(symbol_list) == 0) {
    trailing_characters.push_back(list[symbol_position]);
    word_end = word_end.substr(++symbol_position);
  }

  DEBUG_MSG(word);
  return true;
}

bool Tokenizer::ReadOneWord(const std::string& list, std::string& word) {

}
