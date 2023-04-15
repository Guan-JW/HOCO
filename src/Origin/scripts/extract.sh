cd ../manipulate
make extract

for file in $(ls $1)
do
    if [ $file == fileYyNO.txt ];then
        continue
    fi
    ../bin/extract $1/$file >> $2
done