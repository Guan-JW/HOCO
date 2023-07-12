CC = g++
CFLAGS = -std=c++17 -O3

SCHEMEDIR = ../../SchemeCollectModule/schemes
SRCDIR = ${SCHEMEDIR}/sequiturBasedEngine
# Specify the target executable name
TARGET = WordCount

# Specify the source files (cpp) and header files (hpp)
SRCS = WordCount.cpp 
HDRS = ../EvaluateModule.hpp ${SCHEMEDIR}/include_list.txt ../../Utils/utils.hpp ../../SchemeCollectModule/CollectModule.hpp ../../CompressModule/CompressModule.hpp

# Generate object file names from source files
SequiturOBJS = $(SRCDIR)/sequitur.o $(SRCDIR)/classes.o $(SRCDIR)/compress.o $(SRCDIR)/arith.o $(SRCDIR)/bitio.o $(SRCDIR)/stats.o
OBJS = $(SRCS:.cpp=.o)

# Default rule to build the target
all: $(TARGET)

# Rule to build the target executable
$(TARGET): $(OBJS) $(SequiturOBJS)
	$(CC) $(CFLAGS) $(OBJS) $(SequiturOBJS) -o $(TARGET)


$(SRCDIR)/%.o: $(SRCDIR)/%.cc $(SRCDIR)/classes.h
	g++ -DPLATFORM_UNIX -O3 -c $< -o $@

$(SRCDIR)/arith.o: $(SRCDIR)/arith.c $(SRCDIR)/arith.h $(SRCDIR)/bitio.h $(SRCDIR)/unroll.i
	gcc -O3 -c $< -o $@

$(SRCDIR)/bitio.o: $(SRCDIR)/bitio.c $(SRCDIR)/bitio.h
	gcc -O3 -c $< -o $@

$(SRCDIR)/stats.o: $(SRCDIR)/stats.c $(SRCDIR)/arith.h $(SRCDIR)/stats.h
	gcc -O3 -c $< -o $@

test:
	make; ./$(SRCDIR)/test.pl
force:
	touch $(SRCDIR)/*.cc $(SRCDIR)/*.c; make

# Rule to compile cpp files into object files
%.o: %.cpp $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule to remove generated files
clean:
	rm -f $(OBJS) $(TARGET)
	rm -f *.o
	rm -f $(SRCDIR)/*.o