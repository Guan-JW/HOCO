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
    string input_file = output_dict + "/origin_comp.dic";
	
	struct stat s {};

    stat(input_file.c_str(), &s);
    size_t input_size = s.st_size;

	stat(argv[1], &s);
	size_t original_size = s.st_size;

    cout << "Compression ratio : "
         << (double)(input_size) * 100 / original_size << "%" << endl;
    return 0;
}