VPATH = src \
				src/base \
				src/lqp \
				src/parser \
				src/pqp \
				src/storage \
				storage_manager

INCLUDEDIR = src
CFLAGS += -std=c++14 \
					-I. -I$(INCLUDEDIR) \
					-fpermissive -Wno-static-float-init \
					-DDEBUG
CC = g++
TARGET = database-manager
OBJDIR = obj

SOURCES = StorageManager.cpp \
					duplicate_finder.cpp \
					main.cpp \
					query_manager.cpp \
					query_node.cpp \
					query_plan_builder.cpp \
					query_runner.cpp \
					query_runner_duplicate_elimination.cpp \
					query_runner_factory.cpp \
					query_runner_natural_join.cpp \
					query_runner_product.cpp \
					query_runner_projection.cpp \
					query_runner_scan.cpp \
					query_runner_selection.cpp \
					query_runner_sort.cpp \
					sql_errors.cpp \
					sql_node.cpp \
					sql_parser.cpp \
					statement.cpp \
					statement_create_table.cpp \
					statement_delete.cpp \
					statement_drop_table.cpp \
					statement_factory.cpp \
					statement_insert.cpp \
					statement_select.cpp \
					storage_adapter.cpp \
					tokenizer.cpp \
					tuple_helper.cpp \
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
