#include"common.h"
#include<ctime>
#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<cstring>
#include<vector>
#include<map>
#include<set>
#include<unordered_map>
#include<string>
#include<list>
#include<sys/time.h>
#include<queue>
#include<algorithm>

#include<unordered_set>

#include<fstream>
#include <fcntl.h>    // for O_BINARY
#include <unistd.h>
#include <sys/times.h>
#include <limits.h>

using namespace std;


struct BlockOffset {
    ulong char_offset;
    ulong ele_offset;   // if larger than 32767 then bootstrap
    ulong hash_numerator;
};

struct Ele {
    int id;
    ulong local_char_offset;
    Ele(){id=0; local_char_offset=0;}
};

struct RULE {
    vector<Ele> eleIdx;
};

unordered_map<ulong,string> dictionary_use; // word id - string 
struct RULE* rule_full;
int words, rules;
int splitNum;
ulong* rule_char_length;    // character length of each rule except for the root rule
vector<vector<BlockOffset>> block_off;  // each rule holds an block offset array

int *splitLocation;
set<int> splitSet;

void initialization_basic(char* argv, unordered_map<ulong,string> & dictionary_use, 
        struct RULE*& rule_full, int & words, int & rules){
  char dictionaryDir[100];
  char inputDir[100];
  char relationDir[100];
  sprintf(dictionaryDir,"%s/dictionary.dic",argv);
  sprintf(inputDir,"%s/rowCol.dic",argv);
  sprintf(relationDir, "%s/fileYyNO.txt", argv);
// file split position
  ifstream frelation(relationDir);
  frelation >> splitNum;
  int *split = new int[splitNum];
  memset(split, 0, splitNum);
  splitLocation = new int[splitNum];
  memset(splitLocation, 0, splitNum);
  int tmp;
  for (int i = 0; i < splitNum; i++) {
    frelation >> tmp >> split[i];
  }
  frelation.close();

  ifstream fin(dictionaryDir);
  ulong tem_num;
  string tem_word;
  while(!fin.eof()){
    tem_num=-1;
    fin>>tem_num;

    fin.get();
    tem_word=getWordDictionary(fin);
    dictionary_use[tem_num]=tem_word;
    if (tem_word == string(" ") || tem_word == string("\t") ||
        tem_word == string("\n")) {
        splitSet.insert(tem_num);
    }
  }
  fin.close();

  ifstream finput(inputDir);
  int fileid; finput>>fileid;
  finput>>words>>rules;
  rule_full=new struct RULE[rules];
  block_off.resize(rules + splitNum);

  for(int i=0; i<rules; i++){
    vector<int> st; // rule elements
    int ruleSize;
    finput>>ruleSize;
    for(int j=0; j<ruleSize; j++){
      Ele tempt;
      finput>>tempt.id;
      rule_full[i].eleIdx.push_back(tempt);
    }
  }
  finput.close();

  int splitCur = 0;
  for (int j = 0; j < rule_full[0].eleIdx.size(); j++) {
    int tempt = rule_full[0].eleIdx[j].id;
    if (tempt == split[splitCur] && splitCur < splitNum) {
        splitLocation[splitCur] = j;
        splitCur++;
    }
  }
}

void dfs(int rule, int segment) {
    int rulenum = rule - words;
    ulong global_char_offset = 0;
    ulong local_char_offset = 0;
    ulong hash_numerator = 0;
    vector<BlockOffset> rule_offset;
    for (int i = 0; i < rule_full[rulenum].eleIdx.size(); i++) {
        if(i != 0 && i % segment == 0) {    // skip 0,0
            BlockOffset tempt;
            global_char_offset += local_char_offset;
            tempt.char_offset = global_char_offset; // char offset inside the rule
            tempt.ele_offset = i;   // element offset inside the rule
            tempt.hash_numerator = hash_numerator;
            rule_offset.push_back(tempt);
            local_char_offset = 0;
            hash_numerator = 0;
        }
        int eleId = rule_full[rulenum].eleIdx[i].id;
        if(eleId < words) {
            rule_full[rulenum].eleIdx[i].local_char_offset = local_char_offset;
            local_char_offset += dictionary_use[eleId].length();
            hash_numerator += dictionary_use[eleId].length() * (i % segment);
        }
        else {
            if(rule_char_length[eleId-words-1] == 0) {
                dfs(eleId, segment);
            }
            rule_full[rulenum].eleIdx[i].local_char_offset = local_char_offset;
            local_char_offset += rule_char_length[eleId-words-1];
            hash_numerator += rule_char_length[eleId-words-1] * (i % segment);
        }
    }
    // add the end offset
    BlockOffset tempt;
    global_char_offset += local_char_offset;
    tempt.char_offset = global_char_offset; // char offset inside the rule
    tempt.ele_offset = rule_full[rulenum].eleIdx.size();   // element offset inside the rule
    tempt.hash_numerator = hash_numerator;
    rule_offset.push_back(tempt);
    block_off[rulenum + splitNum] = rule_offset;

    rule_char_length[rulenum-1] = global_char_offset;
}

void traversal_basic(struct RULE*& rule_full, int & words, int & rules, int & segment) {
    rule_char_length = new ulong[rules-1];  // skip root rule
    memset(rule_char_length, 0, rules-1);
    int start = 0, end = 0;
    for(int file=0; file<=splitNum; file++) {
        if(file != 0)   
            start = end;
        end = splitLocation[file];
        if(file == splitNum)
            end = rule_full[0].eleIdx.size();
        ulong global_char_offset = 0;
        ulong local_char_offset = 0;
        ulong hash_numerator = 0;
        vector<BlockOffset> rule_offset;
        for(int i=start; i<end; i++) {
            if(i != start && (i-start) % segment == 0) {    // skip 0,0
                BlockOffset tempt;
                global_char_offset += local_char_offset;
                tempt.char_offset = global_char_offset; // char offset inside the file
                tempt.ele_offset = i-start;   // element offset inside the file
                tempt.hash_numerator = hash_numerator;
                rule_offset.push_back(tempt);
                local_char_offset = 0;
                hash_numerator = 0;
            }
            int eleId = rule_full[0].eleIdx[i].id;
            if(eleId < words) {
                rule_full[0].eleIdx[i].local_char_offset = local_char_offset;
                local_char_offset += dictionary_use[eleId].length();
                hash_numerator += dictionary_use[eleId].length() * ((i-start) % segment);
            }
            else {
                if(rule_char_length[eleId-words-1] == 0) {
                    dfs(eleId, segment);
                }
                rule_full[0].eleIdx[i].local_char_offset = local_char_offset;
                local_char_offset += rule_char_length[eleId-words-1];
                hash_numerator += rule_char_length[eleId-words-1] * ((i-start) % segment);
            }
        }
        BlockOffset tempt;
        global_char_offset += local_char_offset;
        tempt.char_offset = global_char_offset; // char offset inside the file
        tempt.ele_offset = end-start;   // element offset inside the file
        tempt.hash_numerator = hash_numerator;
        rule_offset.push_back(tempt);

        block_off[file] = rule_offset;
    }
}

int main(int argc, char** argv){

    clock_t t1, t2, t3;
    struct Ele tmp;
    int segment = 4096 / sizeof(tmp);
    t1 = clock();
    initialization_basic(argv[1], dictionary_use, rule_full, words, rules);
    t2 = clock();
    traversal_basic(rule_full, words, rules, segment);
	t3 = clock();

    // write to file
    char offsetPath[100];
    char block_off_Path[100];
    sprintf(offsetPath,"%s/offset.txt",argv[1]);
    sprintf(block_off_Path,"%s/block_offset.txt",argv[1]);
    ofstream off_file(offsetPath);
    ofstream blockoff_file(block_off_Path);
    for(int i=0; i < rules; i++) {
        for(int j=0; j < rule_full[i].eleIdx.size(); j++) {
            off_file << rule_full[i].eleIdx[j].local_char_offset << " ";
        }
        off_file << endl;
    }
    for(int i=0; i < rules + splitNum; i++) {
        for(int j=0; j < block_off[i].size(); j++) {
            blockoff_file << block_off[i][j].char_offset << " " 
                    << block_off[i][j].ele_offset << " " 
                    << block_off[i][j].hash_numerator << " ";
        }
        blockoff_file << endl;
    }
    off_file.close();
    blockoff_file.close();

	cout << "IO(s): " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl;
    cout << "Computation(s): " << (double)(t3 - t2) / CLOCKS_PER_SEC << endl;
    cout << "Total(s): " << (double)(t3 - t1) / CLOCKS_PER_SEC << endl;

  return 0;
}