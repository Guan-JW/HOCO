FLAGS= -O3 -g -std=c++11
EXEDIR=./bin

all: extract insert delete 

extract: extract.cpp basics.o
	g++ ${FLAGS} basics.cpp extract.cpp -o ${EXEDIR}/extract
	rm -f *.o

insert: insert.cpp basics.o
	g++ ${FLAGS} basics.cpp insert.cpp -o ${EXEDIR}/insert
	rm -f *.o

delete: delete.cpp basics.o
	g++ ${FLAGS} basics.cpp delete.cpp -o ${EXEDIR}/delete
	rm -f *.o

basics: ${HEADFILES}/basics.cpp ${HEADFILES}/basics.h
	g++ -O3 -std=c++11 -c ${HEADFILES}/basics.cpp 

clean:
	rm -f *.o ${EXEDIR}/extract ${EXEDIR}/insert ${EXEDIR}/delete 
