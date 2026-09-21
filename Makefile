.PHONY: all build run clean

# Build directory
BUILD_DIR := build

# Executable path
EXEC := $(BUILD_DIR)/bin/game

all: build run

build:
	@echo "==> Creating build directory..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. && cmake --build .

run:
	@echo "==> Running the game..."
	@$(EXEC)

clean:
	@echo "==> Cleaning build directory..."
	@rm -rf $(BUILD_DIR)
