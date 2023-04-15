#include "basics.h"

extern ulong* split_words;
extern ulong* split_locations;
extern ulong split_number;
extern map<int, string> dic;
using namespace std;


void initialize(const char* fname,
                vector<vector<BlockOffset>>& block_off, 
             vector<vector<vector<Ele>>>& rule_full) {
    char name[1024];
    FILE *fp_dic, *fp_file;
    strcpy(name, fname);
    strcat(name, "/file.bin");
    fp_file = fopen(name, "rb");
    if (!fp_file)
        cout << name << " "
             << " cannot open" << endl;
    
    strcpy(name, fname);
    strcat(name, "/dictionary.txt");
    fp_dic = fopen(name, "rb");
	if(!fp_dic)
        cout << name << " "
             << "cannot open" << endl;
    int dicSize;
    int nouse;
    fread(&dicSize, sizeof(int), 1, fp_dic);
    fread(&nouse, sizeof(int), 1, fp_dic);
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

    strcpy(name, fname);
    strcat(name, "/block_offset.txt");
    ifstream fblock(name);

    strcpy(name, fname);
    strcat(name, "/offset.txt");
    ifstream foffset(name);
    
    for (int i = 0; i < split_number; i++) {
        string line;
        vector<BlockOffset> bo;
        if (fblock.eof()) {
            cout << "block Error!!" << endl;
            exit(0);
        }
        getline(fblock, line);
        istringstream ss(line);
        ulong char_offset;
        ulong ele_offset;
        ulong old_ele_offset = 0;
        ulong hash_numerator = 0;
        vector<vector<Ele>> rule_block_full;
        while (ss >> char_offset >> ele_offset >> hash_numerator) {
            BlockOffset tempt;
            tempt.char_offset = char_offset;
            tempt.ele_offset = ele_offset;
            tempt.hash_numerator = hash_numerator;
            tempt.rfid = i;
            bo.push_back(tempt);
            vector<Ele> block_full;
            for (ulong j = old_ele_offset; j < ele_offset; j++) {
                Ele tempt;
                if (feof(fp_file)) {
                    cout << "input Error!! " << endl;
                    exit(0);
                }
                fread(&(tempt.id), sizeof(int), 1, fp_file);

                if (foffset.eof()) {
                    cout << "offset Error!! old=" << old_ele_offset << "; new=" << ele_offset << endl;
                    exit(0);
                }
                foffset >> tempt.local_char_offset;
                block_full.push_back(tempt);
            }
            rule_block_full.push_back(block_full);
            old_ele_offset = ele_offset;
        }
        rule_full.push_back(rule_block_full);
        block_off.push_back(bo);
    }
    if (feof(fp_file))  cout << "Read to the end" << endl;
    fblock.close();
    foffset.close();
    fclose(fp_file);
}