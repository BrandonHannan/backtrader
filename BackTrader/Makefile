# The C++ compiler to use
CXX = g++

# Compiler flags. -std=c++20 for the standard, -Wall for all warnings.
CXXFLAGS = -std=c++20 -Wall -g #-arch x86_64 -g

# For Apple Silicon Macs (M1/M2/M3)
# BOOST_INCLUDE_PATH = -I/usr/local/include
# BOOST_LIB_PATH = -L/usr/local/lib

# For Windows
VCPKG_ROOT = C:/path/to/your/vcpkg
# BOOST_INCLUDE_PATH = -I$(VCPKG_ROOT)/installed/x64-windows/include
# BOOST_LIB_PATH = -L$(VCPKG_ROOT)/installed/x64-windows/lib

# The name of your final executable file
TARGET = a

# The source files for your program
SRCS = Main.cpp DataReader/datareader.cpp PositionType/Position.cpp PositionType/PositionType.cpp TradingStrategy/BaseStrategy.cpp TradingStrategy/Custom/CustomStrategy.cpp TradingContext/BaseContext.cpp TradingContext/BreakoutContext/BreakoutContext.cpp TradingContext/Functions/InverseNormalCDF.cpp TradingContext/Functions/StringHelper.cpp TradingContext/Functions/WindowStatistics.cpp PositionSizing/BasePositionSize.cpp PositionSizing/ATRPositionSize/ATRPositionSize.cpp

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
