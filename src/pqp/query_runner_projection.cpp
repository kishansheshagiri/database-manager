#include "pqp/query_runner_projection.h"

#include <string>
#include <vector>

#include "base/debug.h"

QueryRunnerProjection::QueryRunnerProjection(QueryNode *query_node)
  : QueryRunner(query_node) {
}

QueryRunnerProjection::~QueryRunnerProjection() {
}

bool QueryRunnerProjection::Run(QueryResultCallback callback,
    SqlErrors::Type& error_code) {
  if (Node() == nullptr || Node()->ChildrenCount() != 1) {
    DEBUG_MSG("");
    error_code = SqlErrors::ERROR_PROJECTION;
    return false;
  }

  SetCallback(callback);

  QueryNode *child_node = Node()->Child(0);
  SetChildRunner(Create(child_node));

  return ChildRunner()->Run(
      std::bind(&QueryRunnerProjection::ResultCallback,
          this, std::placeholders::_1, std::placeholders::_2),
      error_code);
}

bool QueryRunnerProjection::ResultCallback(std::vector<Tuple>& tuples,
    bool headers) {
  std::vector<std::string> select_list;
  if (!Node()->SelectList(select_list)) {
    DEBUG_MSG("");
    return false;
  }

  if (select_list.size() == 1 && select_list[0] == "*") {
    return Callback()(tuples, headers);
  }

  std::string temp_relation_name;
  if (!Storage()->CreateDummyRelation("Projection_", select_list,
      temp_relation_name)) {
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

  if (!Callback()(output_tuples, headers)) {
    DEBUG_MSG("");
    return false;
  }

  Storage()->DeleteDummyRelation(temp_relation_name);
  return true;
}
