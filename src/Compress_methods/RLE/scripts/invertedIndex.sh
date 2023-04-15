cd ../rleAnalyze

if [ ! -d ./bin ];then
    mkdir ./bin
fi
make invertedIndex

# echo $2
# echo $1
./bin/invertedIndex $1 >> $2