RM = rm -rf

CC=g++

SRC_DIR=./src
OBJ_DIR=./build
BIN_DIR=./bin

CFLAGS = -Wall -pedantic -ansi -std=c++17

all: dir driver

otimize: CFLAGS += -ftracer -Ofast
otimize: dir driver

debug: CFLAGS += -g -O0 -pg
debug: driver

driver: $(OBJ_DIR)/driver.o
	$(CC) $(CFLAGS) -o $(BIN_DIR)/$@ $^

$(OBJ_DIR)/driver.o: $(SRC_DIR)/driver.cpp
	$(CC) -c $(CFLAGS) -o $@ $<	

dir:
	mkdir -p bin build results
	
clean: dir
	$(RM) $(BIN_DIR)/*
	$(RM) $(OBJ_DIR)/*