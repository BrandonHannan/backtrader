# The C++ compiler to use
CXX = g++

# Compiler flags. -std=c++20 for the standard, -Wall for all warnings.
CXXFLAGS = -std=c++20 -Wall -g -I./inlcude #-arch x86_64 -g

# For Apple Silicon Macs (M1/M2/M3)
# BOOST_INCLUDE_PATH = -I/usr/local/include
# BOOST_LIB_PATH = -L/usr/local/lib

# For Windows
VCPKG_ROOT = C:/path/to/your/vcpkg
# BOOST_INCLUDE_PATH = -I$(VCPKG_ROOT)/installed/x64-windows/include
# BOOST_LIB_PATH = -L$(VCPKG_ROOT)/installed/x64-windows/lib

# The name of your final executable file
TARGET = a

# Define a custom function to recursively find files
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

# The source files for your program
SRCS = $(call rwildcard, ., *.cpp)

# --- Makefile Rules ---

# The default rule, executed when you just type "make"
all: $(TARGET)

# Rule to create the executable from the source files
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(BOOST_INCLUDE_PATH) -o $(TARGET) $(SRCS)

# $(BOOST_LIB_PATH) $(BOOST_LIBS)

# Rule to run the program
run: all
	./$(TARGET)

# Rule to clean up the build files
clean:
	if exist $(TARGET).exe del $(TARGET).exe
