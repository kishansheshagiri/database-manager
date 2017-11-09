VPATH = src src/base src/parser src/query_plan src/storage storage_manager
INCLUDEDIR = src
CFLAGS += -std=c++14 -I. -I$(INCLUDEDIR) -Wno-static-float-init -DDEBUG
CC = g++
TARGET = database-manager
OBJDIR = obj

SOURCES = StorageManager.cpp \
					main.cpp \
					query_manager.cpp \
					sql_errors.cpp \
					sql_node.cpp \
					sql_parser.cpp \
					statement.cpp \
					storage_adapter.cpp \
					tokenizer.cpp

OBJECTS := $(patsubst %.cpp, %.o, $(SOURCES))

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $^ -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(TARGET)
