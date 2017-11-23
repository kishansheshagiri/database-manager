#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "base/debug.h"
#include "base/sql_errors.h"
#include "query_manager.h"

void TrimCarriageReturn(std::string& line) {
  if (*line.rbegin() == '\r') {
    line.erase(line.length() - 1);
  }
}

int main(int argc, char *argv[]) {
  logging::DisableConsoleLogs();

  // Start query Manager
  QueryManager *query_manager = QueryManager::Get();

  std::ifstream command_file;
  std::istream *input_stream;
  if (argc > 1) {
    std::ios_base::iostate mask = command_file.exceptions() | std::ios::failbit;
    command_file.exceptions(mask);
    try {
      command_file.open(argv[1]);
    }
    catch (std::ios_base::failure& e) {
      ERROR_MSG("Unable to open file; check the path. Error: " << e.what());
      return 1;
    }

    input_stream = &command_file;
  } else {
    input_stream = &std::cin;
  }

  std::string user_command;
  ERROR_MSG_SINGLE_LINE("TinySQL> ");

  // Main run loop
  while (std::getline(*input_stream, user_command)) {
    TrimCarriageReturn(user_command);

    // Set Query
    SqlErrors::Type error_code = SqlErrors::NO_ERROR;
    if (command_file.is_open()) {
      ERROR_MSG_SINGLE_LINE(user_command << std::endl);
    }

    query_manager->SetQuery(user_command, error_code);
    if (error_code == SqlErrors::EMPTY_STATEMENT) {
      ERROR_MSG_SINGLE_LINE("TinySQL> ");
      continue;
    }

    // Start executing query
    query_manager->ExecuteQuery(error_code);

    ERROR_MSG(error_code << std::endl);
    ERROR_MSG_SINGLE_LINE("TinySQL> ");
  }

  if (command_file.is_open()) {
    command_file.close();
    ERROR_MSG(std::endl);
  }

  delete query_manager;
  return 0;
}
