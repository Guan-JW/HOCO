#include "basics.h"

extern ulong* split_words;
extern ulong* split_locations;
extern ulong split_number;
extern vector<rule> rules;
using namespace std;


bool a_less_b(const TremFreq &r, const TremFreq &s) { return (r.n > s.n); }

bool a_less_b_dic(const TremFreq &r, const TremFreq &s) { return (r.word < s.word); }

int getC0(int word) {
    if (word >= DICMAXSIZE) // if is word
        return word;
    else
        return getC0(rules[word].p); // return the referred rule / ok, c is the current word.
}


LzwInfo init(const char* fname, vector<string>& dic,
                vector<vector<BlockOffset>>& block_off, 
             vector<vector<vector<Ele>>>& rule_full) {

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


    int dicSize, totalSize;
    fread(&dicSize, sizeof(int), 1, fp_dic);
    fread(&totalSize, sizeof(int), 1, fp_dic);

    int ruleSize;
    fread(&ruleSize, sizeof(int), 1, fp_dic);
    rules.resize(ruleSize);

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

    int ruleID = 0, code = input[0];
    rule r{}; // temp rule

    r.p = code; // data[0] to code?
    for (int i = 1; i < totalSize; i++) {  
        code = input[i]; // count code
        if (code < DICMAXSIZE && code >= ruleID) { // if not found 
            r.c = getC0(r.p); // c = old[0]
        } else // if found
            r.c = getC0(code); // c = dic[0]

        rules[ruleID++] = r; // record rule

        r.p = code; // old
    }

    fclose(fp_file);
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
                tempt.id = input[j];
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
    fblock.close();
    foffset.close();

    return {dicSize, totalSize, ruleSize, input};
}

void get_dic_rev(vector<string>& dic, map<string,int> &dic_rev) {
    for (int i = 0; i < dic.size(); i++) {
        dic_rev[dic[i]] = i;
    }
}

void get_rules_rev(map<vector<int>, int>& rules_rev) {
    for (int i = 0; i < rules.size(); i++) {
        vector<int> tmp(2);
        tmp[0] = rules[i].p;
        tmp[1] = rules[i].c;
        rules_rev[tmp] = i;
    }
}