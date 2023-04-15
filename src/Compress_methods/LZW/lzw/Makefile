all: lzw_compress lzw_decompress 

lzw_compress: compress.cc
	g++ -O3 -std=c++11 compress.cc -o lzw_compress

lzw_decompress: decompress.cc
	g++ -O3 -std=c++11 decompress.cc -o lzw_decompress

clean:
	rm -f lzw_decompress lzw_compress
