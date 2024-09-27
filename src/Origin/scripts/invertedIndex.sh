cd ../analytic
exe_dir=../bin
if [ ! -d "$exe_dir" ]; then
  mkdir "$exe_dir"
fi
make invertedIndex

for file in $(ls $1)
do
    if [ $file == fileYyNO.txt ];then
        continue
    fi
    ${exe_dir}/invertedIndex $1/$file $1/fileYyNO.txt >> $2
done