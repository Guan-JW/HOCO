cd ../analytic
make sequenceCount

for file in $(ls $1)
do
    if [ $file == fileYyNO.txt ];then
        continue
    fi
    ../bin/sequenceCount $1/$file >> $2
done