#include <iostream>
#include <sstream>
#include <string>

#include "query_manager.h"

int main(int argc, char *argv[]) {
  // Start query Manager
  QueryManager *query_manager = QueryManager::Get();

  // Main run loop
  while (true) {
    std::string new_line;
    std::cout << "TinySQL> ";
    std::getline(std::cin, new_line);

    // Set Query.
    bool query_success = true;
    query_manager->SetQuery(new_line, query_success);
    if (!query_success) return 0;

    // Start executing query
    query_manager->ExecuteQuery(query_success);
    if (!query_success) {
      std::cout << "Execution failed, check syntax.";
      return 0;
    }
    std::cout << std::endl << std::endl;
  }

  delete query_manager;
  return 0;
}
