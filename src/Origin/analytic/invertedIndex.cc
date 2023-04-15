
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <sys/stat.h> 
#include <unistd.h>

#include <map>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <ctime>

#include "sys/time.h"
using namespace std;

vector<int> input;
map<string, int> wordDic;
vector<string> dic;
vector<vector<int>> iiDic;
// unordered_map<string, vector<int>> iiDic;
ulong split_number;
ulong* split_words;

string getWord(FILE *fp) {
    char word[99999];
    char *ptr = word;
    char c;                        // current char
    static bool indicator = false; // flags
    static char old;               // last char
    string myStr;
    if (indicator == true) { // last time meet a normal char
        indicator = false;
        return string(1, old); // return the old char as a string
    }
    // every time get a single char
    while ((c = getc(fp)) != EOF) {
        if (!(c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == ',' ||
              c == '.')) {
            // if not a boundary just return the char
            *ptr++ = c;       // add c to the buffer
            indicator = true; // set the indicator
        } else {
            // arrive the boundary c between words, last time not meet a normal
            // char
            if (indicator == false)
                return string(1, c);
            // if have met a boundary, last time meet a normal char
            old = c;
            *ptr++ = '\0'; // end the string (not include the boundary chars)
            ptr = word;    // why not just return string(word) ?
            return string(ptr);
        }
    }
    return myStr; // when meet the end, return a null string
}

int main(int argc, char **argv) {
    clock_t t1, t2, t3;
    t1 = clock();

    char name[1024];
    sprintf(name, "%s", argv[2]);
    ifstream fin_split(name);
    
    if(!fin_split)
        cout << "file " << name <<  "not found !!" << endl;
    fin_split >> split_number;

    split_words = new ulong[split_number];
	int temp;
	for(int i = 0; i < split_number; i ++){
		fin_split >> temp >> split_words[i];
    }

    char relationDir[100];
    sprintf(relationDir,"%s",argv[1]);
    
    FILE *fp;
    if ((fp = fopen(relationDir, "r")) == NULL) {
        cout << "wrong" << endl;
        return 0;
    }

    // initialize
    string myStr;
    int no = 0;
    while ((myStr = getWord(fp)).size() > 0) {
        int x;
        if (wordDic.find(myStr) == wordDic.end()) {
            x = no;
            wordDic[myStr] = no;
            dic.push_back(myStr);
            no ++;
        }
        else    
            x = wordDic[myStr];
        input.push_back(x);
    }
    iiDic.resize(no);

    t2 = clock();
    ulong split_cur = 0;
    map<int, int> word_temp;
    for (int i = 0; i < input.size(); i ++ ) {
        int x = input[i];
        if (split_cur < split_number && x == split_words[split_cur]) {
            for( auto it : word_temp ) {
                if (iiDic[it.first].empty()) {
                    vector<int> v;
                    v.push_back(split_cur);
                    iiDic[it.first] = v;
                }
                else {
                    iiDic[it.first].push_back(split_cur);
                }
            }
            split_cur ++;
            word_temp.clear();
        }
        else {
            if (word_temp.find(x) == word_temp.end()) {
                word_temp[x] = 1;
            }
        }
    }
    

    t3 = clock();

	cout << "IO(s): " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl;
    cout << "Computation(s): " << (double)(t3 - t2) / CLOCKS_PER_SEC << endl;
    cout << "Total(s): " << (double)(t3 - t1) / CLOCKS_PER_SEC << endl;

}