all: wordCount sequenceCount invertedIndex

FLAGS= -O3 -g -std=c++11
HEADFILES=../headers/
EXEDIR=../bin
SOURCEDIR=../src/

wordCount: wordCount.cc
	g++ ${FLAGS} -c wordCount.cc
	g++ ${FLAGS} wordCount.o -o ${EXEDIR}/wordCount
	rm -f wordCount.o 

sequenceCount: sequenceCount.cc
	g++ ${FLAGS} -c sequenceCount.cc
	g++ ${FLAGS} sequenceCount.o -o ${EXEDIR}/sequenceCount
	rm -f sequenceCount.o 

invertedIndex: invertedIndex.cc
	g++ ${FLAGS} -c invertedIndex.cc
	g++ ${FLAGS} invertedIndex.o -o ${EXEDIR}/invertedIndex
	rm -f invertedIndex.o 

clean:
	rm -f ../bin/wordCount ../bin/sequenceCount ../bin/invertedIndex

