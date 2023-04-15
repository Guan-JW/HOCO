cd ../manipulate
make insert

for file in $(ls $1)
do
    if [ $file == fileYyNO.txt ];then
        continue
    fi
    ../bin/insert $1/$file >> $2
done