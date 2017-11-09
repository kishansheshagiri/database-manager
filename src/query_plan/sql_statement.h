#ifndef SRC_SQL_STATEMENT_H
#define SRC_SQL_STATEMENT_H

#include <string>
#include <vector>

class SqlStatement {
 public:
  SqlStatement();
  virtual ~SqlStatement();

  std::vector<std::string> Tables();
  void SetTables(std::vector<std::string> names);

 private:
  std::vector<std::string> tables_;
};

#endif // SRC_SQL_STATEMENT_H
