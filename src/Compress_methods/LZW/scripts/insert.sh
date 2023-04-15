cd ../lzwManipulate

if [ ! -d ./bin ];then
    mkdir ./bin
fi
make insert

# echo $2
# echo $1
./bin/insert $1 >> $2