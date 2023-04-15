cd ../manipulate
make delete

for file in $(ls $1)
do
    if [ $file == fileYyNO.txt ];then
        continue
    fi
    ../bin/delete $1/$file >> $2
done