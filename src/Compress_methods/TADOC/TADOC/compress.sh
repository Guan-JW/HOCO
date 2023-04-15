# echo program: $0 
# echo input: $1
# echo output: $2
# echo Please note that this process could take several minutes.
./bin/getdic $1 $2
# ./bin/numeric $1 $2
# ./bin/sequitur -b ./output/ < ./output/0
./bin/sequitur -b $2 < $2/output.txt
./bin/showcrate $1 $2
# rm ./output/0
# ./bin/sequitur -c $1 
# mv rowCol.dic $2/
# a=`grep ^uU data_original/input.txt`
# echo $a|wc -w > $2/fileYyNO.txt
# for i in $a; do grep $i data_compressed/dictionary.dic | awk -F uU '{print $2 " " $1}'>>$2/fileYyNO.txt; done
# mv output/* $2/
# rmdir output
