all: compress compress_origin

compress: compress.cc
	g++ -O3 -std=c++11 compress.cc -o rle_compress

compress_origin: compress_origin.cc
	g++ -O3 -std=c++11 compress_origin.cc -o rle_compress_origin

clean:
	rm -f rle_compress rle_compress_origin *.o
