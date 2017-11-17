VPATH = src \
				src/base \
				src/lqp \
				src/parser \
				src/pqp \
				src/storage \
				storage_manager

INCLUDEDIR = src
CFLAGS += -std=c++14 -I. -I$(INCLUDEDIR) -Wno-static-float-init -DDEBUG
CC = g++
TARGET = database-manager
OBJDIR = obj

SOURCES = StorageManager.cpp \
					create_table_statement.cpp \
					delete_statement.cpp \
					drop_table_statement.cpp \
					insert_statement.cpp \
					main.cpp \
					query_manager.cpp \
					select_statement.cpp \
					sql_errors.cpp \
					sql_node.cpp \
					sql_parser.cpp \
					statement.cpp \
					statement_factory.cpp \
					storage_adapter.cpp \
					tokenizer.cpp \
					where_clause_helper.cpp \
					where_clause_helper_delete.cpp \
					where_clause_helper_select.cpp

OBJECTS := $(patsubst %.cpp, %.o, $(SOURCES))

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $^ -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(TARGET)
