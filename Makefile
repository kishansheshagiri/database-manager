VPATH = src storage-manager
CFLAGS += -std=c++14 -I. -Wno-static-float-init
CC = g++
TARGET = database-manager
OBJDIR = obj

SOURCES = main.cpp \
					query_manager.cpp \
					sql_parser.cpp \
					StorageManager.cpp

OBJECTS := $(patsubst %.cpp, %.o, $(SOURCES))

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $^ -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(TARGET)
