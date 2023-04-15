cnt_path=$(pwd)
cd ../../
path=$(pwd)
cd $cnt_path

input_folder="$path/input/Original_data"
input_datasets=$(ls $input_folder)
output_folder="$path/input/Compressed_data"


if [ ! -d "$subfolder" ];then
    mkdir $output_folder
fi

for algo in RLE LZW TADOC
do
    subfolder=$output_folder/$algo
    if [ ! -d "$subfolder" ];then
        mkdir $subfolder
    fi

    for input_dataset in ${input_datasets}
    do
        echo "Compressing $input_dataset ..."
        echo ""
        
        out_folder=$subfolder/$input_dataset
        if [ ! -d "$out_folder" ];then
            mkdir $out_folder
        fi

        cp $input_folder/$input_dataset/fileYyNO.txt $out_folder
        cd $path/src/Compress_methods/$algo/scripts/
        bash compress.sh $input_folder/$input_dataset $out_folder
    done
done
