cnt_path=$(pwd)
cd ../../
path=$(pwd)
cd $cnt_path

input_folder="$path/input/Compressed_data"
origin_folder="$path/input/Original_data"
input_datasets=$(ls $origin_folder)
Algo_folder="$path/src/Compress_methods"
Algos=$(ls $Algo_folder)
output_folder="$path/results"

for input_dataset in ${input_datasets}
do
    echo "Manipulating $input_dataset ..."
    echo ""

    subfolder=$output_folder/$input_dataset

    if [ ! -d "$subfolder" ];then
        mkdir $subfolder
    fi
    
    for man in extract insert delete
    do
        if [ $man != extract ];then
            continue
        fi
        output_file=$subfolder/$man.txt
        echo "======= $man operation =======" > $output_file
        for algo in $Algos
        do 
            echo "------- with $algo -------" >> $output_file
            cd $Algo_folder/$algo/scripts/
            bash $man.sh $input_folder/$algo/$input_dataset $output_file
            echo "" >> $output_file
        done

        echo "------- with Uncompressed -------" >> $output_file
        cd $path/src/Origin/scripts/
        bash $man.sh $origin_folder/$input_dataset $output_file
        echo "" >> $output_file
    done
done