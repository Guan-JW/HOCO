cd ../rleAnalyze

if [ ! -d ./bin ];then
    mkdir ./bin
fi
make wordCount

# echo $2
# echo $1
./bin/wordCount $1 >> $2