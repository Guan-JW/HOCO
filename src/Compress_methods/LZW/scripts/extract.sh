cd ../lzwManipulate

if [ ! -d ./bin ];then
    mkdir ./bin
fi
make extract

# echo $2
# echo $1
./bin/extract $1 >> $2