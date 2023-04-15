#include "basics.h"

extern vector<Ele> input;
extern ulong* split_words;
extern ulong* split_locations;
extern ulong split_number;
using namespace std;

bool a_less_b(const TremFreq &r, const TremFreq &s) { return (r.word < s.word); }


void initialize(const char* fname, vector<string>& dic) {
    char name[1024];
    FILE *fp_dic, *fp_file;
    strcpy(name, fname);
    strcat(name, "/file_org.txt");
    ifstream finput(name);
    
    strcpy(name, fname);
    strcat(name, "/dictionary_org.txt");
    fp_dic = fopen(name, "rb");
	if(!fp_dic)
        cout << name << " "
             << "cannot open" << endl;

    strcpy(name, fname);
    strcat(name, "/fileYyNO.txt");
    ifstream fin_split(name);
	if(!fin_split)
        cout << "file" << name <<  "not found !!" << endl;

    fin_split >> split_number;

	split_words = new ulong[split_number];
	split_locations = new ulong[split_number];
	int temp;
	for(int i = 0; i < split_number; i ++){
		fin_split >> temp >> split_words[i];
    }
	fin_split.close();

    int dicSize;
    int total_ele_num;
    fread(&dicSize, sizeof(int), 1, fp_dic);
    fread(&total_ele_num, sizeof(int), 1, fp_dic);

    dic.resize(dicSize);

    char wordStr[10000];
    for (int i = 0; i < dicSize; i ++) {
        int word, size;
        fread(&word, sizeof(int), 1, fp_dic);
        fread(&size, sizeof(int), 1, fp_dic);
        fread(wordStr, sizeof(char), size, fp_dic);
        wordStr[size] = 0;
        dic[word] = string(wordStr, size);
    }
    fclose(fp_dic);

    for (int i = 0; i < total_ele_num; i ++ ) {
        Ele tmp;
        finput >> tmp.id;
        finput >> tmp.length;
        input.push_back(tmp);
    }
    finput.close();
}