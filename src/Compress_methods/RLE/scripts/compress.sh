cd ../rle

make clean
make all

for file in $(ls $1)
do
    if [ $file == fileYyNO.txt ];then
        continue
    fi
    ./rle_compress $1/$file $2
    ./rle_compress_origin $1/$file $2
done