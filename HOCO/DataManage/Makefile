CC = g++
CFLAGS = -std=c++17 -O3

# Specify the target executable name
TARGET = data

# Specify the source files (cpp) and header files (hpp)
SRCS = data.cpp
HDRS = CompressData.hpp DataPath.hpp ../Utils/utils.hpp

# Generate object file names from source files
OBJS = $(SRCS:.cpp=.o)

# Default rule to build the target
all: $(TARGET)

# Rule to build the target executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)


# Rule to compile cpp files into object files
%.o: %.cpp $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule to remove generated files
clean:
	rm -f $(OBJS) $(TARGET)