#include "basics.h"

extern ulong* split_words;
extern ulong* split_locations;
extern ulong split_number;
using namespace std;


bool a_less_b(const TremFreq &r, const TremFreq &s) { return (r.n > s.n); }

bool a_less_b_dic(const TremFreq &r, const TremFreq &s) { return (r.word < s.word); }

LzwInfo init(const char* fname, vector<string>& dic) {

    char name[1024];
    FILE *fp_dic, *fp_file;
    strcpy(name, fname);
    strcat(name, "/file.bin");
    fp_file = fopen(name, "rb");

	if(!fp_file)
        cout << name << " "
             << " cannot open" << endl;

    strcpy(name, fname);
    strcat(name, "/dictionary.txt");
    fp_dic = fopen(name, "rb");

	if(!fp_dic)
        cout << name << " "
             << "cannot open" << endl;

    strcpy(name, fname);
    strcat(name, "/fileYyNO.txt");

    ifstream fin_split(name);

    // # ifdef debug 
	if(!fin_split)
        cout << "file" << name <<  "not found !!" << endl;
// #endif

    fin_split >> split_number;

	split_words = new ulong[split_number];
	split_locations = new ulong[split_number];
	int temp;
	for(int i = 0; i < split_number; i ++){
		fin_split >> temp >> split_words[i];
    }

	fin_split.close();


    int dicSize, totalSize;
    fread(&dicSize, sizeof(int), 1, fp_dic);
    fread(&totalSize, sizeof(int), 1, fp_dic);

    int ruleSize;
    fread(&ruleSize, sizeof(int), 1, fp_dic);

    dic.resize(dicSize);

    char wordStr[10000];
    for (int i = 0; i < dicSize; i++) {
        int id, size;
        fread(&id, sizeof(int), 1, fp_dic);
        fread(&size, sizeof(int), 1, fp_dic);
        fread(wordStr, sizeof(char), size, fp_dic);
        wordStr[size] = 0;
        dic[id - DICMAXSIZE] = string(wordStr, size);
    }
    fclose(fp_dic);

    int* input = (int*)malloc(sizeof(int) * totalSize);
    fread(input, sizeof(int), totalSize, fp_file);

    fclose(fp_file);

    return {dicSize, totalSize, ruleSize, input};
}