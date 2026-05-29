CXX      := g++
CXXFLAGS := -std=c++20 -O3  -Wall -Wextra -static
INCLUDES := -Iinclude

SRC_DIR  := src
OBJ_DIR  := build
TARGET   := engine

# Automatically find all .cpp files in src/
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# ── Default target ────────────────────────────────────────────────────────────
all: $(TARGET)

# ── Link ──────────────────────────────────────────────────────────────────────
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# ── Compile each .cpp → build/*.o ─────────────────────────────────────────────
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# ── Create build dir if missing ───────────────────────────────────────────────
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# ── Debug build (no optimisation, symbols for gdb) ───────────────────────────
debug: CXXFLAGS := -std=c++20 -O0 -g -Wall -Wextra
debug: $(TARGET)

# ── Run the engine ────────────────────────────────────────────────────────────
run: all
	./$(TARGET)

# ── Windows cross-compilation (requires mingw-w64) ────────────────────────────
WIN_CXX     := x86_64-w64-mingw32-g++
WIN_OBJ_DIR := build-win
WIN_TARGET  := $(TARGET).exe
WIN_OBJS    := $(patsubst $(SRC_DIR)/%.cpp, $(WIN_OBJ_DIR)/%.o, $(SRCS))

win: $(WIN_TARGET)

$(WIN_TARGET): $(WIN_OBJS)
	$(WIN_CXX) $(CXXFLAGS) -o $@ $^ -static

$(WIN_OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(WIN_OBJ_DIR)
	$(WIN_CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(WIN_OBJ_DIR):
	mkdir -p $(WIN_OBJ_DIR)

# ── Clean ─────────────────────────────────────────────────────────────────────
clean:
	rm -rf $(OBJ_DIR) $(WIN_OBJ_DIR) $(TARGET) $(WIN_TARGET)

.PHONY: all debug run win clean
