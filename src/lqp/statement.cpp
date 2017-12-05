#include "lqp/statement.h"

Statement::Statement(const SqlNode *root_node)
    : root_node_(root_node),
      print_stats_(false) {
  storage_adapter_ = StorageAdapter::Get();
}

Statement::~Statement() {
  if (print_stats_) {
    storage_adapter_->PrintDiskStats();
  }
}

void Statement::PrintDiskStats() {
  print_stats_ = true;
  storage_adapter_->ResetDiskStats();
}

std::vector<std::string> Statement::Tables() const {
  return tables_;
}

bool Statement::SetTables(const std::vector<std::string> tables) {
  tables_ = tables;
  return true;
}
