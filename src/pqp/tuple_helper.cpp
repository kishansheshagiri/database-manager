#include "pqp/tuple_helper.h"

#include "base/debug.h"

CompareTuples::CompareTuples(const QueryRunner *query_runner,
      std::vector<Tuple> tuples)
    : query_runner_(query_runner),
      tuples_(tuples){ }

bool CompareTuples::operator()(size_t first_index, size_t second_index) const {
  if (first_index >= tuples_.size() || second_index >= tuples_.size()) {
    DEBUG_MSG("");
    return false;
  }

  Tuple first = tuples_[first_index];
  Tuple second = tuples_[second_index];
  if (first.isNull()) {
    return false;
  } else if (second.isNull()) {
    return true;
  }

  std::string sort_column = query_runner_->CompareColumn();
  Schema schema_first = first.getSchema();
  Schema schema_second = second.getSchema();

  if (sort_column == "*" || sort_column.empty()) {
    int index = 0;
    while (index < schema_first.getNumOfFields() &&
        index < schema_second.getNumOfFields()) {
      if (schema_first.getFieldType(index) == schema_second.getFieldType(
          index)) {
        sort_column = schema_first.getFieldName(index);
        break;
      }
    }
  }

  if (sort_column.empty()) {
    return false;
  }

  if (schema_first.getFieldType(sort_column) == INT) {
    int field_value_first = first.getField(sort_column).integer;
    int field_value_second = second.getField(sort_column).integer;

    return field_value_first < field_value_second;
  } else if (schema_first.getFieldType(sort_column) == STR20) {
    std::string field_value_first = *(first.getField(sort_column).str);
    std::string field_value_second = *(second.getField(sort_column).str);

    return field_value_first < field_value_second;
  }

  return true;
}

bool CompareTuples::operator()(Tuple first, Tuple second) const {
  if (first.isNull()) {
    return false;
  } else if (second.isNull()) {
    return true;
  }

  std::string sort_column = query_runner_->CompareColumn();
  Schema schema_first = first.getSchema();
  Schema schema_second = second.getSchema();

  if (sort_column == "*" || sort_column.empty()) {
    int index = 0;
    while (index < schema_first.getNumOfFields() &&
        index < schema_second.getNumOfFields()) {
      if (schema_first.getFieldType(index) == schema_second.getFieldType(
          index)) {
        sort_column = schema_first.getFieldName(index);
        break;
      }
    }
  }

  if (sort_column.empty()) {
    return false;
  }

  if (schema_first.getFieldType(sort_column) == INT) {
    int field_value_first = first.getField(sort_column).integer;
    int field_value_second = second.getField(sort_column).integer;

    return field_value_first < field_value_second;
  } else if (schema_first.getFieldType(sort_column) == STR20) {
    std::string field_value_first = *(first.getField(sort_column).str);
    std::string field_value_second = *(second.getField(sort_column).str);

    return field_value_first < field_value_second;
  }

  return true;
}

bool CompareTuples::IsFieldEqual(size_t first_index,
    size_t second_index) const {
  Tuple first = tuples_[first_index];
  Tuple second = tuples_[second_index];

  if (first.isNull() || second.isNull()) {
    return false;
  }

  std::string sort_column = query_runner_->CompareColumn();
  Schema schema_first = first.getSchema();
  Schema schema_second = second.getSchema();

  if (sort_column == "*" || sort_column.empty()) {
    int index = 0;
    while (index < schema_first.getNumOfFields() &&
        index < schema_second.getNumOfFields()) {
      if (schema_first.getFieldType(index) == schema_second.getFieldType(
          index)) {
        sort_column = schema_first.getFieldName(index);
        break;
      }
    }
  }

  if (sort_column.empty()) {
    return false;
  }

  if (schema_first.getFieldType(sort_column) == INT) {
    int field_value_first = first.getField(sort_column).integer;
    int field_value_second = second.getField(sort_column).integer;

    return field_value_first == field_value_second;
  } else if (schema_first.getFieldType(sort_column) == STR20) {
    std::string field_value_first = *(first.getField(sort_column).str);
    std::string field_value_second = *(second.getField(sort_column).str);

    return field_value_first == field_value_second;
  }

  return false;
}

bool operator ==(const Tuple &first, const Tuple &second) {
  if (first.isNull() && second.isNull()) {
    return true;
  }

  if (first.isNull() || second.isNull()) {
    return false;
  }

  Schema schema_first = first.getSchema();
  Schema schema_second = second.getSchema();
  if (!(schema_first == schema_second)) {
    return false;
  }

  for (auto index = 0; index < schema_first.getNumOfFields(); index++) {
    if (schema_first.getFieldType(index) == INT) {
      int field_value_first = first.getField(index).integer;
      int field_value_second = second.getField(index).integer;

      if (field_value_first != field_value_second) {
        return false;
      }
    } else if (schema_first.getFieldType(index) == STR20) {
      std::string field_value_first = *(first.getField(index).str);
      std::string field_value_second = *(second.getField(index).str);

      if (field_value_first != field_value_second) {
        return false;
      }
    }

    return true;
  }

  return true;
}
