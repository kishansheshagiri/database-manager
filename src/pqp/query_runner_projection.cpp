#include "pqp/query_runner_projection.h"

#include <ctime>
#include <vector>
#include <string>

#include "base/debug.h"

QueryRunnerProjection::QueryRunnerProjection(QueryNode *query_node)
  : QueryRunner(query_node),
    child_runner_(nullptr) {
}

QueryRunnerProjection::~QueryRunnerProjection() {
  if (child_runner_) {
    delete child_runner_;
    child_runner_ = nullptr;
  }
}

bool QueryRunnerProjection::Run(QueryResultCallback callback,
    SqlErrors::Type& error_code) {
  if (Node() == nullptr || Node()->ChildrenCount() != 1) {
    DEBUG_MSG("");
    error_code = SqlErrors::ERROR_PROJECTION;
    return false;
  }

  callback_ = callback;

  QueryNode *child_node = Node()->Child(0);
  child_runner_ = Create(child_node);

  return child_runner_->Run(
      std::bind(&QueryRunnerProjection::ResultCallback,
          this, std::placeholders::_1),
      error_code);
}

bool QueryRunnerProjection::ResultCallback(std::vector<Tuple>& tuples) {
  std::vector<std::string> select_list;
  if (!Node()->SelectList(select_list)) {
    DEBUG_MSG("");
    return false;
  }

  std::time_t time_now = std::time(nullptr);
  std::string temp_relation_name = "Projection" + std::to_string(time_now);
  std::vector<enum FIELD_TYPE> field_types(select_list.size(), STR20);

  if (!Storage()->CreateRelation(temp_relation_name,
      select_list, field_types)) {
    DEBUG_MSG("");
    return false;
  }

  std::vector<Tuple> output_tuples;
  for (auto tuple : tuples) {
    std::vector<std::string> tuples_row;
    for (auto field : select_list) {
      if (tuple.getSchema().getFieldType(field) == INT) {
        int value = tuple.getField(field).integer;
        std::string integer_value = std::to_string(value);
        if (value == -1) {
          integer_value = "NULL";
        }

        tuples_row.push_back(integer_value);
      } else if (tuple.getSchema().getFieldType(field) == STR20) {
        tuples_row.push_back(*(tuple.getField(field).str));
      } else {
        DEBUG_MSG("");
        return false;
      }
    }

    bool tuple_created = false;
    Tuple output_tuple = Storage()->CreateTuple(
        temp_relation_name, tuples_row, tuple_created);
    if (!tuple_created) {
      DEBUG_MSG("");
      return false;
    }

    output_tuples.push_back(output_tuple);
  }

  if (!callback_(output_tuples)) {
    DEBUG_MSG("");
    return false;
  }

  Storage()->DeleteRelation(temp_relation_name);
  return true;
}
