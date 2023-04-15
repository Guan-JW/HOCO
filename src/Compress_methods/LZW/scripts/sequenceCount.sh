cd ../lzwAnalyze

if [ ! -d ./bin ];then
    mkdir ./bin
fi
make sequenceCount

# echo $2
# echo $1
./bin/sequenceCount $1 >> $2