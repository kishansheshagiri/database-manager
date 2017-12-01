#include "base/sql_errors.h"

std::ostream &operator <<(std::ostream &out, const SqlErrors::Type type) {
  switch (type) {
    case SqlErrors::NO_ERROR:
      out << "Query Success";
      break;
    case SqlErrors::UNACCEPTABLE_STATEMENT:
      out << "Statement unacceptable.";
      break;
    case SqlErrors::ATTRIBUTE_TOO_LONG:
      out << "Attribute is too long. Limit: 8";
      break;
    case SqlErrors::FIELD_TOO_LONG:
      out << "Field is too long. Limit: 8";
      break;
    case SqlErrors::INVALID_CHARACTER:
      out << "Contains invalid charater/s";
      break;
    case SqlErrors::INVALID_COLUMN_NAME:
      out << "Column name contains invalid characters";
      break;
    case SqlErrors::INVALID_TABLE_NAME:
      out << "Invalid table name";
      break;
    case SqlErrors::INVALID_TABLE_LIST:
      out << "List of tables is invalid";
      break;
    case SqlErrors::INVALID_SELECT_LIST:
      out << "List of selection attributes is invalid";
      break;
    case SqlErrors::AMBIGUOUS_SELECT_LIST:
      out << "Statement contains ambiguous column names";
      break;
    case SqlErrors::INVALID_ATTRIBUTES:
      out << "Attribute list is invalid";
      break;
    case SqlErrors::INVALID_VALUES:
      out << "Entered values are invalid";
      break;
    case SqlErrors::INVALID_SEARCH_CONDITION:
      out << "Invalid search condition";
      break;
    case SqlErrors::INVALID_SORT_ATTRIBUTE:
      out << "Sort column is invalid";
      break;
    case SqlErrors::WHERE_CLAUSE_ERROR:
      out << "Error processing WHERE condition";
      break;
    case SqlErrors::ERROR_TABLE_SCAN:
      out << "Error reading table contents";
      break;
    case SqlErrors::ERROR_SORT:
      out << "Error in sorting step";
      break;
    case SqlErrors::ERROR_DUPLICATE_ELIMINATION:
      out << "Error in removing duplicate elements";
      break;
    case SqlErrors::ERROR_PROJECTION:
      out << "Error in handling projection step";
      break;
    case SqlErrors::ERROR_SELECTION:
      out << "Error in handling selection step";
      break;
    case SqlErrors::ERROR_CROSS_PRODUCT:
      out << "Error in calculating product table";
      break;
    case SqlErrors::ERROR_NATURAL_JOIN:
      out << "Error in handling join step";
      break;
    case SqlErrors::INEXISTENT_TABLE:
      out << "Table does not exist";
      break;
    case SqlErrors::TABLE_EXISTS:
      out << "Table already exists";
      break;
    case SqlErrors::DUPLICATE_TUPLE:
      out << "Duplicate tuple";
      break;
    case SqlErrors::TUPLE_TOO_LONG:
      out << "Tuple longer than available fields";
      break;
    case SqlErrors::TOKENIZER_NOT_INITIALIZED:
      out << "Tokenizer not initialized";
      break;
    case SqlErrors::TOKENIZER_INVALID_INDEX:
      out << "Invalid tokenizer index";
      break;
    case SqlErrors::TOKENIZER_INVALID_CHARACTER:
      out << "Invalid character found by tokenizer";
      break;
    case SqlErrors::TOKENIZER_EMPTY_INPUT:
      out << "Tokenizer input is empty";
      break;
    case SqlErrors::UNKNOWN_ERROR:
    default:
      out << "Unknown error";
  }

  return out;
}
