all: insert gen_offset extract delete

FLAGS= -O3 -g -std=c++11
HEADFILES=../headers
EXEDIR=../bin
SOURCEDIR=../src

gen_offset: gen_offset.cc
	g++ ${FLAGS} -I${HEADFILES} -c gen_offset.cc
	g++ ${FLAGS} -I${HEADFILES}  -c ${SOURCEDIR}/common.cc
	g++ ${FLAGS} gen_offset.o common.o -o ${EXEDIR}/gen_offset
	rm -f gen_offset.o common.o 

insert: insert.cc
	g++ ${FLAGS} -I${HEADFILES} -c insert.cc
	g++ ${FLAGS} -I${HEADFILES}  -c ${SOURCEDIR}/basic.cc
	g++ ${FLAGS} -I${HEADFILES}  -c ${SOURCEDIR}/common.cc
	g++ ${FLAGS} insert.o basic.o common.o -o ${EXEDIR}/insert
	rm -f insert.o basic.o common.o 

extract: extract.cc
	g++ ${FLAGS} -I${HEADFILES} -c extract.cc
	g++ ${FLAGS} -I${HEADFILES}  -c ${SOURCEDIR}/basic.cc
	g++ ${FLAGS} -I${HEADFILES}  -c ${SOURCEDIR}/common.cc
	g++ ${FLAGS} extract.o basic.o common.o -o ${EXEDIR}/extract
	rm -f extract.o basic.o common.o 

delete: delete.cc
	g++ ${FLAGS} -I${HEADFILES} -c delete.cc
	g++ ${FLAGS} -I${HEADFILES}  -c ${SOURCEDIR}/basic.cc
	g++ ${FLAGS} -I${HEADFILES}  -c ${SOURCEDIR}/common.cc
	g++ ${FLAGS} delete.o basic.o common.o -o ${EXEDIR}/delete
	rm -f delete.o basic.o common.o 

clean:
	rm -rf ${EXEDIR}/delete ${EXEDIR}/extract ${EXEDIR}/insert ${EXEDIR}/late_insert

