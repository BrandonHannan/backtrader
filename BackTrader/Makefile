# The C++ compiler to use
CXX = g++

# Compiler flags. -std=c++20 for the standard, -Wall for all warnings.
CXXFLAGS = -std=c++20 -Wall -arch x86_64 -g

# For Apple Silicon Macs (M1/M2/M3)
BOOST_INCLUDE_PATH = -I/usr/local/include
BOOST_LIB_PATH = -L/usr/local/lib

# For Windows
# VCPKG_ROOT = C:/path/to/your/vcpkg
# BOOST_INCLUDE_PATH = -I$(VCPKG_ROOT)/installed/x64-windows/include
# BOOST_LIB_PATH = -L$(VCPKG_ROOT)/installed/x64-windows/lib

# Libraries to link. Add more as needed, e.g., -lboost_thread
# Note: For the p-value code, linking isn't strictly necessary, but this is the correct way to do it.
BOOST_LIBS = -lboost_system

# The name of your final executable file
TARGET = a

# The source files for your program
SRCS = Main.cpp DataReader/datareader.cpp Position/Position.cpp TradingStrategy/TradingStrategy.cpp TradingStrategy/CustomChannelBreakout/CustomChannelBreakout.cpp

# --- Makefile Rules ---

# The default rule, executed when you just type "make"
all: $(TARGET)

# Rule to create the executable from the source files
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(BOOST_INCLUDE_PATH) -o $(TARGET) $(SRCS) $(BOOST_LIB_PATH) $(BOOST_LIBS)

# Rule to run the program
run: all
	./$(TARGET)

# Rule to clean up the build files
clean:
	rm -f $(TARGET)
