FLAGS= -O3 -g -std=c++11
EXEDIR=bin

all: sequenceCount wordCount invertedIndex

sequenceCount: sequenceCount.cpp basics.o 
	g++ ${FLAGS} sequenceCount.cpp basics.cpp -o ${EXEDIR}/sequenceCount 
	rm -f *.o

wordCount: wordCount.cpp basics.o 
	g++ ${FLAGS} wordCount.cpp basics.cpp -o ${EXEDIR}/wordCount
	rm -f *.o

invertedIndex: invertedIndex.cpp basics.o 
	g++ ${FLAGS} invertedIndex.cpp basics.cpp -o ${EXEDIR}/invertedIndex
	rm -f *.o

basics: basics.cpp basics.h 
	g++ -O3 -std=c++11 -c basics.cpp

clean:
	rm -f *.o ${EXEDIR}/sequenceCount ${EXEDIR}/wordCount ${EXEDIR}/invertedIndex
	rm -f *.o sequenceCount wordCount
