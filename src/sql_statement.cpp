#include "sql_statement.h"

SqlStatement::SqlStatement() {

}

SqlStatement::~SqlStatement() {

}

std::vector<std::string> SqlStatement::Tables() {
  return tables_;
}

void SqlStatement::SetTables(std::vector<std::string> names) {
  tables_ = names;
}
