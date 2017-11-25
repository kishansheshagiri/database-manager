#ifndef SRC_SQL_ERRORS_H
#define SRC_SQL_ERRORS_H

#include <ostream>

struct SqlErrors {
  enum Type {
    NO_ERROR = 0,
    EMPTY_STATEMENT = 100,
    UNACCEPTABLE_STATEMENT,
    ATTRIBUTE_TOO_LONG,
    FIELD_TOO_LONG,
    INVALID_CHARACTER,
    INVALID_COLUMN_NAME,
    INVALID_TABLE_NAME,
    INVALID_TABLE_LIST,
    INVALID_SELECT_LIST,
    AMBIGUOUS_SELECT_LIST,
    INVALID_ATTRIBUTES,
    INVALID_VALUES,
    INVALID_SEARCH_CONDITION,

    INEXISTENT_TABLE = 200,
    TABLE_EXISTS,

    DUPLICATE_TUPLE = 300,
    TUPLE_TOO_LONG,

    TOKENIZER_NOT_INITIALIZED = 900,
    TOKENIZER_INVALID_INDEX,
    TOKENIZER_INVALID_CHARACTER,
    TOKENIZER_EMPTY_INPUT,

    UNKNOWN_ERROR = 1000
  };

  friend std::ostream &operator <<(std::ostream &out, const Type type);
};

#endif //SRC_SQL_ERRORS_H
