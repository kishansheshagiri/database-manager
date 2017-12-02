#include "pqp/query_runner_product.h"

#include <vector>

#include "base/debug.h"
#include "pqp/where_clause_helper_select.h"

QueryRunnerProduct::QueryRunnerProduct(QueryNode *query_node)
  : QueryRunner(query_node),
    error_code_(SqlErrors::NO_ERROR) {
}

QueryRunnerProduct::~QueryRunnerProduct() {
}

bool QueryRunnerProduct::Run(QueryResultCallback callback,
    SqlErrors::Type& error_code) {
  if (Node() == nullptr || Node()->ChildrenCount() != 2) {
    DEBUG_MSG("");
    error_code = SqlErrors::ERROR_CROSS_PRODUCT;
    return false;
  }

  SetCallback(callback);

  QueryNode *left_child = Node()->Child(0);
  QueryNode *right_child = Node()->Child(1);
  if (right_child->Type() == QueryNode::QUERY_NODE_TYPE_CROSS_PRODUCT) {
    SetChildRunner(Create(right_child));
    table_scan_child_ = Create(left_child);
  } else {
    SetChildRunner(Create(left_child));
    table_scan_child_ = Create(right_child);
  }

  std::string table_name_first, table_name_second;
  ChildRunner()->TableName(table_name_first);
  table_scan_child_->TableName(table_name_second);

  std::vector<Tuple> headers_first, headers_second;
  ChildRunner()->TableHeaders(headers_first);
  table_scan_child_->TableHeaders(headers_second);

  std::vector<Tuple> merged_headers;
  if (!MergeTableHeaders(headers_first, table_name_first,
      headers_second, table_name_second, merged_headers)) {
    DEBUG_MSG("");
    return false;
  }

  ScanParams params;
  params.headers_disabled_ = true;
  if (ChildRunner()->NodeType() != QueryNode::QUERY_NODE_TYPE_CROSS_PRODUCT) {
    params.num_blocks_ = 1;
  }

  ChildRunner()->PassScanParams(params);

  Callback()(this, merged_headers, true);

  if (!ChildRunner()->Run(
      std::bind(&QueryRunnerProduct::ResultCallback, this,
          std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
      error_code)) {
    DEBUG_MSG("");
    if (error_code_ == SqlErrors::NO_ERROR) {
      error_code_ = SqlErrors::ERROR_CROSS_PRODUCT;
    }

    error_code = error_code_;
    return false;
  }

  return true;
}

bool QueryRunnerProduct::ResultCallback(QueryRunner *child,
    std::vector<Tuple>& tuples, bool headers) {
  if (child == ChildRunner()) {
    ScanParams params;
    if (ChildRunner()->NodeType() != QueryNode::QUERY_NODE_TYPE_CROSS_PRODUCT) {
      params.num_blocks_ = 1;
      params.use_begin_blocks_ = false;
    }

    ChildRunner()->PassScanParams(params);

    if (!table_scan_child_->Run(
        std::bind(&QueryRunnerProduct::ResultCallback, this,
            std::placeholders::_1, std::placeholders::_2,
            std::placeholders::_3),
        error_code_)) {
      DEBUG_MSG("");
      error_code_ = SqlErrors::ERROR_CROSS_PRODUCT;
      return false;
    }
  } else if (child == table_scan_child_) {

  } else {
    DEBUG_MSG("");
    error_code_ = SqlErrors::ERROR_CROSS_PRODUCT;
    return false;
  }

  return true;
}
