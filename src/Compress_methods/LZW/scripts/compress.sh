cd ../lzw

make clean
make all

for file in $(ls $1)
do
    if [ $file == fileYyNO.txt ];then
        continue
    fi
    ./lzw_compress $1/$file $2
done