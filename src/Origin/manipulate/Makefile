all: insert extract delete

FLAGS= -O3 -g -std=c++11
HEADFILES=../headers/
EXEDIR=../bin
SOURCEDIR=../src/

insert: insert.cc
	g++ ${FLAGS} -c insert.cc
	g++ ${FLAGS} insert.o -o ${EXEDIR}/insert
	rm -f insert.o 
	
extract: extract.cc
	g++ ${FLAGS} -c extract.cc
	g++ ${FLAGS} extract.o -o ${EXEDIR}/extract
	rm -f extract.o 

delete: delete.cc
	g++ ${FLAGS} -c delete.cc
	g++ ${FLAGS} delete.o -o ${EXEDIR}/delete
	rm -f delete.o 

clean:
	rm -f ${EXEDIR}/insert ${EXEDIR}/extract ${EXEDIR}/delete

