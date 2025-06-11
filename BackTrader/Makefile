# The C++ compiler to use
CXX = g++

# Compiler flags. -std=c++20 for the standard, -Wall for all warnings.
CXXFLAGS = -std=c++20 -Wall

# The name of your final executable file
TARGET = a

# The source file for your program
SRCS = Main.cpp

# --- Makefile Rules ---

# The default rule, executed when you just type "make"
all: $(TARGET)

# Rule to create the executable from the source file
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

# Rule to run the program
run: all
	./$(TARGET)

# Rule to clean up the build files
clean:
	rm -f $(TARGET)