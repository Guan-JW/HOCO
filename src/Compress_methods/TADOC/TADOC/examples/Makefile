all: wordCount invertedIndex sequenceCount

FLAGS= -O3 -g -std=c++11
HEADFILES=../headers
EXEDIR=../bin
SOURCEDIR=../src

wordCount: wordCount.cc
	g++ ${FLAGS} -I${HEADFILES} -c wordCount.cc
	g++ ${FLAGS} -I${HEADFILES}  -c ${SOURCEDIR}/basic.cc
	g++ ${FLAGS} -I${HEADFILES}  -c ${SOURCEDIR}/common.cc
	g++ ${FLAGS} wordCount.o basic.o common.o -o ${EXEDIR}/wordCount
	rm -f wordCount.o basic.o common.o 

invertedIndex: invertedIndex.cc
	g++ ${FLAGS} -I${HEADFILES} -c invertedIndex.cc
	g++ ${FLAGS} -I${HEADFILES} -c ${SOURCEDIR}/fileBottomUp.cc
	g++ ${FLAGS} -I${HEADFILES}  -c ${SOURCEDIR}/common.cc
	g++ ${FLAGS} invertedIndex.o fileBottomUp.o common.o  -o ${EXEDIR}/invertedIndex
	rm -f invertedIndex.o fileBottomUp.o common.o 

sequenceCount: sequenceCount.cc
	g++ ${FLAGS} -I${HEADFILES} -c sequenceCount.cc
	g++ ${FLAGS} -I${HEADFILES} -c ${SOURCEDIR}/sequenceWithCount.cc
	g++ ${FLAGS} -I${HEADFILES}  -c ${SOURCEDIR}/common.cc
	g++ ${FLAGS} sequenceCount.o sequenceWithCount.o common.o  -o ${EXEDIR}/sequenceCount
	rm -f sequenceCount.o sequenceWithCount.o common.o

clean:
	rm -rf ${EXEDIR}/invertedIndex ${EXEDIR}/sequenceCount ${EXEDIR}/wordCount

