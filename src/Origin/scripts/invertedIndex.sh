cd ../analytic
make invertedIndex

for file in $(ls $1)
do
    if [ $file == fileYyNO.txt ];then
        continue
    fi
    ../bin/invertedIndex $1/$file $1/fileYyNO.txt >> $2
done