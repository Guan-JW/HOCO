cd ../analytic
exe_dir=../bin
if [ ! -d "$exe_dir" ]; then
  mkdir "$exe_dir"
fi
make wordCount

for file in $(ls $1)
do
    if [ $file == fileYyNO.txt ];then
        continue
    fi
    ${exe_dir}/wordCount $1/$file >> $2
done