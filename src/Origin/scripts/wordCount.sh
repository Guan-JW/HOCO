cd ../analytic
make wordCount

for file in $(ls $1)
do
    if [ $file == fileYyNO.txt ];then
        continue
    fi
    ../bin/wordCount $1/$file >> $2
done