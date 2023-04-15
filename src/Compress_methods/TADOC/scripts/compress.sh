cd ../TADOC/

bash compile.sh

for file in $(ls $1)
do
    if [ $file == fileYyNO.txt ];then
        continue
    fi
    bash compress.sh $1/$file $2
done

cd ./manipulate
make gen_offset
../bin/gen_offset $2