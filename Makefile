# ========= Compiler =========
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# ========= Output =========
TARGET = main.exe

# ========= Directories =========
BUILD_DIR = build

# ========= Source Files =========
SRCS = main.cpp \
       lexer/Lexer.cpp \
       parser/Parser.cpp \
       semantic/Semantic.cpp

# ========= Object Files =========
OBJS = $(SRCS:%.cpp=$(BUILD_DIR)/%.o)

# ========= Include Paths =========
INCLUDES = -Ilexer -Iparser -Isemantic -Icommon

# ========= Default Target =========
all: $(BUILD_DIR) $(TARGET)

# ========= Link =========
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

# ========= Compile =========
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# ========= Create build dir =========
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# ========= Run =========
run: all
	./$(TARGET) test/test1.pl0

# ========= Clean =========
clean:
	rm -rf $(BUILD_DIR) $(TARGET)