.PHONY: all build run test clean

BUILD_DIR := build
EXEC := $(BUILD_DIR)/bin/game
TEST_EXEC := $(BUILD_DIR)/bin/game_tests

all: build run

build:
	@echo "==> Configuring and building..."
	@cmake -S . -B $(BUILD_DIR)
	@cmake --build $(BUILD_DIR)

run: build
	@echo "==> Running the game..."
	@$(EXEC)

test:
	@echo "==> Building and running the headless gameplay checks..."
	@cmake -S . -B $(BUILD_DIR)
	@cmake --build $(BUILD_DIR) --target game_tests
	@$(TEST_EXEC)

clean:
	@echo "==> Cleaning build directory..."
	@rm -rf $(BUILD_DIR)
