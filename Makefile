BUILD_DIR = build
SRC_DIR = src
TEST_DIR = test
DEMO_DIR = demo

CROSS_COMPILE ?=
CC:=$(CROSS_COMPILE)gcc

C_SOURCES_ALL := $(shell tree -if src | grep "\.c")
C_SOURCES := $(filter-out %main.c,$(C_SOURCES_ALL))
C_MAIN := $(SRC_DIR)/main.c
C_OBJECTS := $(patsubst src/%,build/%,$(patsubst %.c,%.o,$(C_SOURCES)))
C_MAIN_OBJECT := $(BUILD_DIR)/main.o

C_INCLUDE=\
	-Iinclude
C_DEFINE?=-D_GNU_SOURCE\
	-DCONFIG_INTERNAL_MEMORY=131072\
	-DCONFIG_DYNAMIC_ERROR_LOG
#	-DCONFIG_USE_RV64
# C_DEFINE?=-D_GNU_SOURCE\
# 	-DCONFIG_DEFAULT_MEMORY_FILE_SIZE=131072
C_LIBS=

OPT_FLAG?=-O3

CFLAGS := $(CFLAGS) -Wall -Wpedantic -Werror -std=c23 $(OPT_FLAG) $(C_INCLUDE) $(C_DEFINE) $(C_LIBS)

TEST_SOURCES := $(wildcard $(TEST_DIR)/*.c)
TEST_OBJECTS := $(patsubst $(TEST_DIR)/%,$(BUILD_DIR)/%,$(patsubst %.c,%.test.o,$(TEST_SOURCES)))
RUNABLE_TESTS := $(patsubst $(BUILD_DIR)/%.test.o,run-test-%,$(TEST_OBJECTS))
TESTS := $(patsubst $(TEST_DIR)/%.c,$(BUILD_DIR)/%.test,$(TEST_SOURCES))

.PHONY: all main clean tests demos demo_helloworld_all demo_helloworld_clean

all: main tests demos

#
# Project Sources
#
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo CC $@
	@$(CC) -o $@ $< $(CFLAGS) -c

#
# Main
#
main: $(BUILD_DIR)/main 
$(BUILD_DIR)/main: $(C_OBJECTS) $(C_MAIN_OBJECT)
	@echo CC $@
	@$(CC) $^ -o $@ $(CFLAGS)

#
# Tests
#
tests: $(TESTS) demos
$(BUILD_DIR)/%.test: $(TEST_DIR)/%.c $(C_SOURCES) 
	@echo CC $@
	@$(CC) $^ $(CFLAGS) -o $@
	@echo TEST $@
	@cd $(BUILD_DIR)/; ../$@
	@rm -rf $@

demos: demo_helloworld_all
demo_helloworld_all:
	@$(MAKE) -C $(DEMO_DIR)/helloworld all

demo_helloworld_clean:
	@$(MAKE) -C $(DEMO_DIR)/helloworld clean

#
# Clean
#
clean: demo_helloworld_clean
	@$(RM) -rf $(BUILD_DIR)
