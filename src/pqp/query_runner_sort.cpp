#include "pqp/query_runner_sort.h"

#include <algorithm>
#include <numeric>
#include <string>
#include <vector>

#include "base/debug.h"
#include "pqp/tuple_helper.h"
#include "storage/storage_manager_headers.h"

typedef struct CompareTuples {
  CompareTuples(const QueryRunnerSort *query_runner, std::vector<Tuple> tuples)
      : query_runner_(query_runner),
        tuples_(tuples){ }
  const QueryRunnerSort *query_runner_;
  const std::vector<Tuple> tuples_;

  bool operator() (size_t first_index, size_t second_index) const {
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

    std::string sort_column = query_runner_->SortColumn();
    Schema schema = first.getSchema();

    if (sort_column == "*" || sort_column.empty()) {
      sort_column = schema.getFieldName(0);
    }

    if (schema.getFieldType(sort_column) == INT) {
      int field_value_first = first.getField(sort_column).integer;
      int field_value_second = second.getField(sort_column).integer;

      return field_value_first < field_value_second;
    } else if (schema.getFieldType(sort_column) == STR20) {
      std::string field_value_first = *(first.getField(sort_column).str);
      std::string field_value_second = *(second.getField(sort_column).str);

      return field_value_first < field_value_second;
    }

    return true;
  }
} CompareTuples;

QueryRunnerSort::QueryRunnerSort(QueryNode *query_node)
  : QueryRunner(query_node),
    block_size_(-1),
    tuples_per_block_(-1),
    memory_constraint_(Storage()->MainMemorySize() - 1),
    scan_params_passed_(false),
    error_code_(SqlErrors::NO_ERROR) {
}

QueryRunnerSort::~QueryRunnerSort() {
}

bool QueryRunnerSort::Initialize(SqlErrors::Type& error_code) {
  if (Node() == nullptr || Node()->ChildrenCount() != 1) {
    DEBUG_MSG("");
    error_code = SqlErrors::ERROR_SORT;
    return false;
  }

  QueryNode *child_node = Node()->Child(0);
  SetChildRunner(Create(child_node));
  if (!Node()->SortColumn(sort_column_)) {
    DEBUG_MSG("");
    return false;
  }

  return ChildRunner()->Initialize(error_code);
}

bool QueryRunnerSort::Run(QueryResultCallback callback,
    SqlErrors::Type& error_code) {
  SetCallback(callback);

  if (scan_params_passed_ && (scan_params_.num_blocks_ == 1 ||
      scan_params_.num_blocks_ + scan_params_.start_index_ - 1 >
          memory_constraint_)) {
    scan_params_.num_blocks_ = memory_constraint_ - scan_params_.start_index_;
    memory_constraint_ = scan_params_.num_blocks_;
  }

  if (!scan_params_passed_) {
    scan_params_.num_blocks_ = memory_constraint_;
  }

  ChildRunner()->PassScanParams(scan_params_);

  if (!ChildRunner()->Run(
      std::bind(&QueryRunnerSort::ResultCallback, this,
          std::placeholders::_1, std::placeholders::_2),
      error_code)) {
    DEBUG_MSG("");
    error_code = SqlErrors::ERROR_SORT;
    return false;
  }

  if (block_size_ >= 0 && block_size_ <= memory_constraint_) {
    return true;
  }

  std::vector<Block *> blocks;
  int memory_index = scan_params_.start_index_;
  int relation_index = 0;
  std::vector<int> sublist_block_indices(sublist_size_list_.size(), 0);
  for (auto index = 0; index < sublist_size_list_.size(); index++) {
    std::vector<Block *> sublist_blocks;
    if (!Storage()->ReadRelationBlocks(intermediate_relation_name_,
        relation_index, memory_index++, 1, sublist_blocks)) {
      DEBUG_MSG("");
      return false;
    }

    sublist_block_indices[index] = relation_index;
    relation_index += sublist_size_list_[index];
    if (index > 0) {
      sublist_size_list_[index] += sublist_size_list_[index - 1];
    }
    blocks.push_back(sublist_blocks[0]);
  }

  Tuple dummy_tuple = Tuple::getDummyTuple();
  dummy_tuple.null();
  Tuple minimum_tuple = dummy_tuple;
  std::vector<int> block_tuple_indices(sublist_size_list_.size(), 0);
  while (true) {
    std::vector<Tuple> minimum_tuples(sublist_size_list_.size(), dummy_tuple);
    for (auto index = 0; index < sublist_block_indices.size(); index++) {
      if (block_tuple_indices[index] == blocks[index]->getNumTuples()) {
        if (sublist_block_indices[index] < sublist_size_list_[index] - 1) {
          std::vector<Block *> sublist_blocks;
          if (!Storage()->ReadRelationBlocks(intermediate_relation_name_,
              ++sublist_block_indices[index], scan_params_.start_index_ + index,
              1, sublist_blocks)) {
            DEBUG_MSG("");
            return false;
          }

          blocks[index] = sublist_blocks[0];
          block_tuple_indices[index] = 0;
        } else {
          block_tuple_indices[index] = -1;
        }
      }

      if (sublist_block_indices[index] < sublist_size_list_[index] &&
          block_tuple_indices[index] > -1) {
        minimum_tuples[index] = blocks[index]->getTuple(
            block_tuple_indices[index]);
      } else {
        minimum_tuples[index].null();
      }
    }

    std::vector<size_t> sort_indices(minimum_tuples.size());
    std::iota(sort_indices.begin(), sort_indices.end(), 0);
    std::sort(sort_indices.begin(),
        sort_indices.end(), CompareTuples(this, minimum_tuples));
    minimum_tuple = minimum_tuples[sort_indices[0]];
    block_tuple_indices[sort_indices[0]]++;

    if (minimum_tuple.isNull()) {
      break;
    }

    std::vector<Tuple> output_tuples;
    output_tuples.push_back(minimum_tuple);
    Callback()(this, output_tuples);
  }

  return true;
}

void QueryRunnerSort::PassScanParams(ScanParams params) {
  scan_params_ = params;
  if (ChildRunner()) {
    ChildRunner()->PassScanParams(params);
  }

  scan_params_passed_ = true;
}

bool QueryRunnerSort::HasSortNode() const {
  return true;
}

bool QueryRunnerSort::ResultCallback(QueryRunner *child,
    std::vector<Tuple>& tuples) {
  if (tuples.empty()) {
    return true;
  }

  if (tuples_per_block_ == -1) {
    tuples_per_block_ = tuples[0].getSchema().getTuplesPerBlock();
  }

  int tuple_size;
  if (TableSize(block_size_, tuple_size) && block_size_ <= memory_constraint_) {
    std::vector<size_t> sort_indices(tuples.size());
    std::iota(sort_indices.begin(), sort_indices.end(), 0);
    std::sort(sort_indices.begin(), sort_indices.end(),
        CompareTuples(this, tuples));

    std::vector<Tuple> sorted_tuples;
    for (auto sort_index = 0; sort_index < sort_indices.size(); sort_index++) {
      sorted_tuples.push_back(tuples[sort_indices[sort_index]]);
    }
    return Callback()(this, sorted_tuples);
  }

  if (intermediate_relation_name_.empty()) {
    if (!createIntermediateRelation(tuples[0])) {
      DEBUG_MSG("");
      error_code_ = SqlErrors::ERROR_SORT;
      return false;
    }
  }

  std::vector<size_t> sort_indices(tuples.size());
  std::iota(sort_indices.begin(), sort_indices.end(), 0);
  std::sort(sort_indices.begin(), sort_indices.end(),
      CompareTuples(this, tuples));

  for (auto sort_index = 0; sort_index < sort_indices.size(); sort_index++) {
    Storage()->AppendTupleUsing(intermediate_relation_name_,
        tuples[sort_indices[sort_index]], Storage()->MainMemorySize() - 1);
  }

  Storage()->PushLastBlock(intermediate_relation_name_, memory_constraint_);
  int block_count = tuples.size() / tuples_per_block_;
  int adjusted_block_count = tuples.size() % tuples_per_block_ ?
      block_count + 1 : block_count;

  sublist_size_list_.push_back(adjusted_block_count);
  if (adjusted_block_count > memory_constraint_ ||
      sublist_size_list_.size() > memory_constraint_) {
    ERROR_MSG("Out of memory");
    return false;
  }

  return true;
}

std::string QueryRunnerSort::SortColumn() const {
  return sort_column_;
}

bool QueryRunnerSort::createIntermediateRelation(Tuple& tuple) {
  Schema schema = tuple.getSchema();
  std::vector<std::string> field_names = schema.getFieldNames();
  std::vector<enum FIELD_TYPE> field_types = schema.getFieldTypes();

  if (!Storage()->CreateDummyRelation("Sort_", field_names, field_types,
      intermediate_relation_name_)) {
    DEBUG_MSG("");
    return false;
  }

  MarkTemporaryRelation(intermediate_relation_name_);

  return true;
}

bool QueryRunnerSort::sublistIterated(std::vector<int>& sublist_size_list,
    std::vector<int>& sublist_block_indices) {
  if (sublist_size_list.size() != sublist_block_indices.size()) {
    return false;
  }

  for (auto index = 0; index < sublist_size_list.size(); index++) {
    if (sublist_block_indices[index] < sublist_size_list[index] - 1) {
      return false;
    }
  }

  return true;
}
