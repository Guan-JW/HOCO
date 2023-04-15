FLAGS= -O3 -g -std=c++11
EXEDIR=./bin

all:  wordCount sort sequenceCount invertedIndex

wordCount: wordCount.cpp basics.o
	g++ ${FLAGS} basics.cpp wordCount.cpp -o ${EXEDIR}/wordCount
	rm -f *.o

sequenceCount: sequenceCount.cpp basics.o
	g++ ${FLAGS} basics.cpp sequenceCount.cpp -o ${EXEDIR}/sequenceCount
	rm -f *.o

invertedIndex: invertedIndex.cpp basics.o
	g++ ${FLAGS} basics.cpp invertedIndex.cpp -o ${EXEDIR}/invertedIndex
	rm -f *.o


clean:
	rm -f *.o ${EXEDIR}/wordCount ${EXEDIR}/sequenceCount ${EXEDIR}/invertedIndex
