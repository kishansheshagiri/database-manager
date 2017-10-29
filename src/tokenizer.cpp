#include "tokenizer.h"

#include "debug.h"
#include "sql_errors.h"

Tokenizer::Tokenizer() {
}

void Tokenizer::CleanupSpaces(std::string& input) {
  std::string space_as_string = "\\s";
  std::regex replace_space(space_as_string +
      "+(?=(?:[^\\\"]*\\\"[^\\\"]*\\\"[^\\\"]*|[^\\\"])*$)");
  input = std::regex_replace(input, replace_space, " ");
}

bool Tokenizer::ReadOneWord(const std::string list,
    std::string& word) {
  size_type space_position = list.find_first_of(", ");
  if (space_position <= 0) {
    return false;
  }

  word = list.substr(0, space_position);
  return true;
}
