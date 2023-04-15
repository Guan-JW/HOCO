cd ../TADOC/examples

if [ ! -d ../bin ];then
    mkdir ../bin
fi
rm -f ../bin/wordCount
make wordCount

# echo $2
# echo $1
../bin/wordCount $1 >> $2