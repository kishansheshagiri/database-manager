VPATH = src storage-manager
CFLAGS += -std=c++11 -I.
CC = g++

OBJECTS := main.o sql_statement.o StorageManager.o

database-manager: $(OBJECTS)
	$(CC) $^ -o $@

.PHONY: clean

clean:
	rm -f $(OBJDIR) *~ core

