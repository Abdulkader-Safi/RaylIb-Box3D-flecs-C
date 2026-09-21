.PHONY: help all build run test check web serve clean

BUILD_DIR := build
WEB_BUILD_DIR := build-web
DIST_DIR := dist
WEB_DIR := $(DIST_DIR)/web
ZIP := $(DIST_DIR)/game-web.zip

EXEC := $(BUILD_DIR)/bin/game
TEST_EXEC := $(BUILD_DIR)/bin/game_tests
SERVE_PORT ?= 8000

# Default target, so a bare `make` explains itself.
help:
	@echo "Twin stick shooter"
	@echo
	@echo "  make build    Build the desktop game"
	@echo "  make run      Build and play it"
	@echo "  make test     Layer check, then the headless gameplay checks"
	@echo "  make check    Layer check on its own (game code must go through core)"
	@echo "  make web      Build for WebAssembly and pack $(ZIP) for itch.io"
	@echo "  make serve    Build for web and serve it on localhost:$(SERVE_PORT)"
	@echo "  make clean    Remove $(BUILD_DIR), $(WEB_BUILD_DIR) and $(DIST_DIR)"
	@echo
	@echo "  make all      Build the desktop game and run it"

all: build run

build:
	@echo "==> Configuring and building..."
	@cmake -S . -B $(BUILD_DIR)
	@cmake --build $(BUILD_DIR)

run: build
	@echo "==> Running the game..."
	@$(EXEC)

test: check
	@echo "==> Building and running the headless gameplay checks..."
	@cmake -S . -B $(BUILD_DIR)
	@cmake --build $(BUILD_DIR) --target game_tests
	@$(TEST_EXEC)

check:
	@echo "==> Checking that game code goes through core..."
	@./tools/check_layers.sh

# Builds a WebAssembly version and packs it the way itch.io wants: a zip with
# index.html at its root. Upload $(ZIP), tick "This file will be played in the
# browser", and set the viewport to 1280 x 720.
#
# Needs Emscripten on PATH (brew install emscripten, or an emsdk you sourced).
web:
	@command -v emcmake >/dev/null 2>&1 || \
		{ echo "emcmake not found. Install Emscripten: brew install emscripten"; exit 1; }
	@echo "==> Configuring the web build..."
	@emcmake cmake -S . -B $(WEB_BUILD_DIR) -DCMAKE_BUILD_TYPE=Release
	@echo "==> Building to WebAssembly..."
	@cmake --build $(WEB_BUILD_DIR)
	@echo "==> Packing $(WEB_DIR)..."
	@rm -rf $(WEB_DIR) && mkdir -p $(WEB_DIR)
	@cp $(WEB_BUILD_DIR)/bin/game.html $(WEB_DIR)/index.html
	@cp $(WEB_BUILD_DIR)/bin/game.js $(WEB_BUILD_DIR)/bin/game.wasm $(WEB_DIR)/
	@rm -f $(ZIP)
	@cd $(WEB_DIR) && zip -q -r ../../$(ZIP) .
	@echo "==> Ready to upload: $(ZIP)"
	@ls -lh $(ZIP) $(WEB_DIR)

# Serves the web build locally. Opening index.html straight off disk will not
# work: browsers refuse to fetch the .wasm over file://.
serve: web
	@echo "==> http://localhost:$(SERVE_PORT)  (ctrl+c to stop)"
	@cd $(WEB_DIR) && python3 -m http.server $(SERVE_PORT)

clean:
	@echo "==> Cleaning build directories..."
	@rm -rf $(BUILD_DIR) $(WEB_BUILD_DIR) $(DIST_DIR)
