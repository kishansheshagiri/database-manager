VPATH = src storage-manager
CFLAGS += -std=c++14 -I. -Wno-static-float-init -DDEBUG
CC = g++
TARGET = database-manager
OBJDIR = obj

SOURCES = StorageManager.cpp \
					main.cpp \
					query_manager.cpp \
					sql_errors.cpp \
					sql_node.cpp \
					sql_parser.cpp \
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