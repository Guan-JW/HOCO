# include <iostream>
# include <fstream>
# include <string>
# include <sys/stat.h> 


using namespace::std;


int main(int argc, char **argv){
	if(argc < 3){
        cout << "please specify the dictionary path." << endl;
    }
	// files
    string output_dict = argv[2];
    string dic_file = output_dict + "/dictionary.dic";
    string input_file = output_dict + "/rowCol.dic";
    string block_file = output_dict + "/block_offset.txt";
	
	struct stat s {};

    stat(dic_file.c_str(), &s);
    size_t dic_length = s.st_size;

    stat(input_file.c_str(), &s);
    size_t output_length = s.st_size;

    stat(block_file.c_str(), &s);
    size_t block_length = s.st_size / 4;

	stat(argv[1], &s);
	size_t original_length = s.st_size;

    cout << "Orginal size : " << original_length << " " << ", Compressed size : " << output_length << ", Dictionary size : " << dic_length << ", Block size : " << block_length << endl;
	cout << "Compression ratio (text+dic) : " << (double)original_length / (output_length + dic_length) << endl;
    cout << "Compression ratio (text+dic+block) : " << (double)original_length / (output_length + dic_length + block_length) << endl;
    cout << "Dictionary occ : " << (double)dic_length / (output_length + dic_length + block_length) << endl;
    cout << "Block occ : " << (double)block_length / (output_length + dic_length + block_length) << endl;
    
    return 0;
}