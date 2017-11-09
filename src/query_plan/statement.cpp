#include "query_plan/statement.h"

Statement::Statement(SqlNode *root_node)
  : root_node_(root_node) {
}

Statement::~Statement() {

}

std::vector<std::string> Statement::Tables() const {
  return tables_;
}

void Statement::SetTables(const std::vector<std::string> names) {
  tables_ = names;
}
