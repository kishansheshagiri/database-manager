#include "tokenizer.h"

#include "debug.h"

Tokenizer::Tokenizer(std::string input_string)
  : input_string_(input_string),
    characters_to_cleanup_(",<>=+-*"),
    tokenized_(false) {
}

void Tokenizer::Tokenize(SqlErrors::Type &error_code) {
  if (input_string_.length() == 0) {
    DEBUG_MSG(__FILE__ << ":" << __LINE__);
    error_code = SqlErrors::TOKENIZER_EMPTY_INPUT;
    return;
  }

  DEBUG_MSG("Input: " << input_string_);

  cleanupOperators();

  std::regex re_tokenize(
      "\\s+(?=(?:[^\\\"]*\\\"[^\\\"]*\\\"[^\\\"]*|[^\\\"])*$)");
  std::sregex_token_iterator
      first{input_string_.begin(), input_string_.end(), re_tokenize, -1},
      last;
  token_list_ = {first, last};

  DEBUG_MSG("Tokens:");
  for (auto printer:token_list_) DEBUG_MSG("\t" << printer);

  tokenized_ = true;
}

void Tokenizer::HandleSelectAll() {
  if (!tokenized_ || token_list_.size() <= 1) {
    DEBUG_MSG(__FILE__ << ":" << __LINE__);
    return;
  }

  std::vector<std::string> select_split;

  // Handle SELECT *
  SplitIntoWords(token_list_[0], "\\*", select_split);
  if (select_split.size() == 2) {
    token_list_[0] = select_split[0];
    token_list_.insert(token_list_.begin() + 1, "*");
    token_list_.insert(token_list_.begin() + 2, select_split[1]);
    DEBUG_MSG(__FILE__ << ":" << __LINE__);
    return;
  }

  // Handle SELECT DISTINCT *
  select_split.clear();
  SplitIntoWords(token_list_[1], "\\*", select_split);
  if (select_split.size() == 2) {
    token_list_[1] = select_split[0];
    token_list_.insert(token_list_.begin() + 2, "*");
    token_list_.insert(token_list_.begin() + 3, select_split[1]);
    DEBUG_MSG(__FILE__ << ":" << __LINE__);
  }
}

int Tokenizer::WordCount(SqlErrors::Type &error_code) {
  if (tokenized_) {
    DEBUG_MSG(__FILE__ << ":" << __LINE__);
    return token_list_.size();
  }

  error_code = SqlErrors::TOKENIZER_NOT_INITIALIZED;
  return -1;
}

std::string Tokenizer::Word(int index, SqlErrors::Type &error_code) {
  if (!tokenized_) {
    DEBUG_MSG(__FILE__ << ":" << __LINE__);
    error_code = SqlErrors::TOKENIZER_NOT_INITIALIZED;
    return std::string();
  }

  if (index < token_list_.size()) {
    DEBUG_MSG(__FILE__ << ":" << __LINE__);
    return token_list_[index];
  }

  error_code = SqlErrors::TOKENIZER_INVALID_INDEX;
  return std::string();
}

// Private
void Tokenizer::cleanupOperators() {
  for (auto &operator_char : characters_to_cleanup_) {
    std::string char_as_string = std::string(1, operator_char);
    std::regex replace_space(" *[\\" + char_as_string +
        "]+ *(?=(?:[^\\\"]*\\\"[^\\\"]*\\\"[^\\\"]*|[^\\\"])*$)");
    input_string_ = std::regex_replace(input_string_,
        replace_space,char_as_string);
  }
}

// Static methods
void Tokenizer::SplitIntoWords(
    const std::string &long_string, const std::string &delimiter,
    std::vector<std::string> &result) {
  std::regex re_split(
      "(" + delimiter + ")+");

  splitIntoWords(long_string, re_split, result);
}

void Tokenizer::SplitIntoWordsMultiDelimiters(
    const std::string &long_string, const std::string &delimiters,
    std::vector<std::string> &result) {
  std::regex re_split(
      "[" + delimiters + "]+");

  splitIntoWords(long_string, re_split, result);
}

void Tokenizer::splitIntoWords(
    const std::string &long_string, const std::regex& split_regex,
    std::vector<std::string> &result) {
  std::sregex_token_iterator
      first{long_string.begin(), long_string.end(), split_regex, -1},
      last;

  result = {first, last};
}
