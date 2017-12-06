#ifndef SRC_PQP_TUPLE_HELPER_H
#define SRC_PQP_TUPLE_HELPER_H

#include <string>
#include <vector>

#include "pqp/query_runner.h"
#include "storage/storage_manager_headers.h"

typedef struct CompareTuples {
  CompareTuples(const QueryRunner *query_runner, std::vector<Tuple> tuples);
  bool operator()(size_t first_index, size_t second_index) const;
  bool operator()(Tuple first, Tuple second) const;
  bool IsFieldEqual(size_t first_index, size_t second_index) const;

 private:
  const QueryRunner *query_runner_;
  const std::vector<Tuple> tuples_;
} CompareTuples;

bool operator ==(const Tuple &first, const Tuple &second);

#endif // SRC_PQP_TUPLE_HELPER_H
