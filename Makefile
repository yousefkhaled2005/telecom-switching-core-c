# ==============================================================================
# Makefile for Telecom Switching Core Engine (SS7 / ISUP / Zero-Copy Routing)
# Lead Systems Architect: Yousef Khaled (2004-2008)
# ==============================================================================

CC ?= gcc
CXX ?= g++
AS ?= as

CFLAGS = -Wall -Wextra -O3 -Iinclude -D_GNU_SOURCE
CXXFLAGS = -Wall -Wextra -O3 -std=c++17 -Iinclude -Icpp
LDFLAGS = -lpthread

SRCS_C = src/ring_buffer.c src/ss7_isup.c src/memory_pool.c src/packet_switch.c src/cdr_processor.c src/main.c
SRCS_CPP = cpp/exchange_monitor.cpp
SRCS_ASM = asm/atomic_ops_x86.s

OBJS = $(SRCS_C:.c=.o) $(SRCS_CPP:.cpp=.o)
BIN_DIR = bin
TARGET = $(BIN_DIR)/telecom_switch_core

.PHONY: all clean release debug test

all: release

release: CFLAGS += -DNDEBUG -march=native
release: $(TARGET)

debug: CFLAGS += -g -DDEBUG -O0
debug: $(TARGET)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(TARGET): $(BIN_DIR) $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@
	@echo "Build complete: $@"

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

cpp/%.o: cpp/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS) $(BIN_DIR)
	@echo "Cleaned build artifacts."

test: release
	./$(TARGET)
