#ifndef SRC_LQP_STATEMENT_H
#define SRC_LQP_STATEMENT_H

#include <string>
#include <vector>

#include "base/sql_errors.h"
#include "parser/sql_node.h"
#include "storage/storage_adapter.h"

class Statement {
 public:
  Statement(const SqlNode *root_node);
  virtual ~Statement();

  virtual void Execute(SqlErrors::Type& error_code) = 0;
  virtual void PrintDiskStats();

 protected:
  const SqlNode *RootNode() const { return root_node_; }
  StorageAdapter *Storage() const { return storage_adapter_; }

  std::vector<std::string> Tables() const;
  virtual bool SetTables(const std::vector<std::string> tables);

 private:
  Statement() {}

  const SqlNode *root_node_;
  bool print_stats_;
  StorageAdapter *storage_adapter_;
  std::vector<std::string> tables_;
};

#endif // SRC_LQP_STATEMENT_H
