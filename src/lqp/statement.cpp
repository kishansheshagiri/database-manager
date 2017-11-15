#include "lqp/statement.h"

Statement::Statement(const SqlNode *root_node)
    : root_node_(root_node) {
  storage_adapter_ = StorageAdapter::Get();
}

Statement::~Statement() {

}

std::vector<std::string> Statement::Tables() const {
  return tables_;
}

bool Statement::SetTables(const std::vector<std::string> tables) {
  tables_ = tables;
  return true;
}
