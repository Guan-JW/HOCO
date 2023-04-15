cd ../TADOC/manipulate

if [ ! -d ../bin ];then
    mkdir ../bin
fi
make delete

# echo $2
# echo $1
../bin/delete $1 >> $2