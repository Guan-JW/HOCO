// #define _GNU_SOURCE

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <sys/stat.h> 
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <map>
#include "flat_hash_map.hpp"
// #include "sys/syscall.h"
#include "sys/time.h"
using namespace std;

#define DICMAXSIZE 1000000000

#define RULEMAX 1000

#define NUMTHREADS 8


struct VectorHasher { 
    int operator()(const vector<int> &V) const { // hash(vector) = vector[0]
        unsigned int hash = 0;
        //     for(int i=0;i<V.size();i++) {
        // //      hash = (hash<<1+V[i])%19260817;
        //       //hash = ((hash<<1)+V[i])%147483647;
        //       //hash = (unsigned long
        //       long)(hash*131+V[i])%212370440130137957ll;
        //       //hash = (hash*131+V[i])%19260817;
        //       hash = hash + V[i]; // Can be anything
        //     }
        return V[0];
    }
};

ska::flat_hash_map<string, int> dic; // hashmap from a string to a interger
map<int, string> dic_rev; // hashmap from a string to a interger
vector<int> input;
int no;
int splitNum;
int *splitLocation;

unordered_map<vector<int>, int, VectorHasher>
        rules; // hashmap from a vector to a interger
unordered_map<int, string>
        rules2word; // hashmap from a interger to a string


// int mainThreadID;
double timestamp() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec + 1e-6 * tv.tv_usec;
}

/*
 * Just get a word....
 * When met a non-boundary char just store it in a buffer
 * Else return the word in buffe and then return the boundary char
 */
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
    // mainThreadID = gettid();// syscall(SYS_gettid);

    double time1 = timestamp();
	
	if(argc <= 2){
		cout << "please specify the input file path." << endl;
		return 0;
	}

    FILE *fp_dic;
	char dic_file[500];
	char output_file[500];
	sprintf(dic_file, "%s/dictionary.txt", argv[2]);
	sprintf(output_file, "%s/file.bin", argv[2]);

    string blockoff_path = argv[2];
    blockoff_path += "/block_offset.txt";
    ofstream fblock(blockoff_path);

    string offset_path = argv[2];
    offset_path += "/offset.txt";
    ofstream foffset(offset_path);

    string rule_path = argv[2];
    rule_path += "/rules.txt";
    ofstream frule(rule_path);

    fp_dic = fopen(dic_file, "wb"); // create a file as dictionary

    FILE *fp_file;
    fp_file = fopen(output_file, "wb"); // create a file as output

    FILE *fp;
    char word[99999];

    char ruletmp[99999];

    if ((fp = fopen(argv[1], "r")) == NULL)
        cout << "wrong" << endl;
        
	struct stat s{};
	stat(argv[1], &s);
	size_t original_length = s.st_size;

    // split
    char relationDir[500];
    sprintf(relationDir, "%s/fileYyNO.txt", argv[2]);
    ifstream frelation(relationDir);
    frelation >> splitNum;
    int *split = new int[splitNum];
    memset(split, 0, splitNum);
    splitLocation = new int[splitNum];
    memset(splitLocation, 0, splitNum);
    int t;
    for (int i = 0; i < splitNum; i++) {
        frelation >> t >> split[i];
    }
    frelation.close();

    string myStr;

    int outputSize = 0; // Size of nums in the output file

    int n = 0;
    char c;
    char *ptr = word;
    no = DICMAXSIZE;

    vector<int> tmp(2);

    int curtmpSize = 0, ruleID = 0;

    int inputSize = 0;

    // here just to make a dictionary
    while ((myStr = getWord(fp)).size() > 0) { // haven't meet the end
        int x = dic[myStr];                    // hash the char or string
        if (x == 0) {                           // if the char doesn't exist
            x = no;
            dic[myStr] = no;
            dic_rev[no] = myStr;
            tmp[0] = no;
            tmp[1] = no;
            rules[tmp] = no;

            no++;
        }

        int cur = x; // current char

        input.push_back(cur);
		
	
    }
    inputSize = input.size();
    vector<int> w;

	double test = timestamp();

    w.push_back(0);

    int splitCur = 0;
    int splitTotal = 0;
    int ruleSize = 0; // Size of rules
    
    int block_size = 4096 / sizeof(int);
    ulong block_charoff = 0;
    ulong block_eleoff = 0;
    ulong block_hash_numerator = 0;

    int cnt_eleoff = 1;
    ulong local_charoffset = 0;
    foffset << local_charoffset << " ";
    bool newfile = false;
    double time3 = timestamp();

    // Here is the lzw compress
    int i = 0;
    for (; i < inputSize; i++) { // word by word
        int c = input[i];                 // c is the i^th word (int)

        vector<int> wc(w);

        wc.push_back(c);
        wc[0] += c; // make the vector hashable

        if (rules.count(wc))
            // c in dic, add it and come to next iteration
            w = wc;
        else {
            // c not in dic, add wc to dic and write w to file
            fwrite(&rules[w], sizeof(int), 1, fp_file); // compressed result
            int length = 0;
            for (int i = 0; i < w.size(); i++) {
                length += dic_rev[w[i] - DICMAXSIZE].length();
            }
            local_charoffset += length;
            foffset << local_charoffset << " "; // write offset
            block_charoff += length;
            block_hash_numerator += length * cnt_eleoff;
            frule << rules[w] << " " << c - DICMAXSIZE << endl;
            rules[wc] = ruleSize++; // become new words // new rule start from 0 
            w.clear();
            outputSize++;
            cnt_eleoff ++;
            if (c - DICMAXSIZE == split[splitCur] && splitCur < splitNum) { // end of current file
                block_eleoff += cnt_eleoff;
                fblock << block_charoff << " " << block_eleoff << " " << block_hash_numerator << " ";
                fblock << endl;
                // reset block list
                block_hash_numerator = 0;
                block_eleoff = 0;
                block_charoff = 0;
                cnt_eleoff = 0;
                local_charoffset = 0;

                splitLocation[splitCur] = block_eleoff;
                splitTotal ++;
                splitCur++;
                if (++i < inputSize)
                    w.push_back(input[i]);    // put next character
            }
            else {
                if (cnt_eleoff == block_size) {    // create new block
                    block_eleoff += cnt_eleoff;
                    fblock << block_charoff << " " << block_eleoff << " " << block_hash_numerator << " ";
                    block_hash_numerator = 0;
                    cnt_eleoff = 0;
                    local_charoffset = 0;
                }
                w.push_back(c);
            }
            
        }
    }
    // if end with a word in dictionary
    if (!w.empty()) {
        fwrite(&rules[w], sizeof(int), 1, fp_file);
        outputSize++;
        cnt_eleoff++;
        block_eleoff += cnt_eleoff;
        fblock << block_charoff << " " << block_eleoff << " " << block_hash_numerator;
        int length = 0;
        for (int i = 0; i < w.size(); i++) {
            length += dic_rev[w[i]].length();
        }
        local_charoffset += length;
        foffset << local_charoffset; // write offset
    }


    fclose(fp);
    fclose(fp_file);
    fblock.close();
    foffset.close();
    frule.close();
    
    int dicSize = dic.size();
    fwrite(&(dicSize), sizeof(int), 1, fp_dic);
    fwrite(&(outputSize), sizeof(int), 1, fp_dic);
    fwrite(&(ruleSize), sizeof(int), 1, fp_dic);
    //  int nthreads = NUMTHREADS ;
    // fwrite( &(nthreads ), sizeof(int), 1, fp_dic);
    for (ska::flat_hash_map<string, int>::iterator i = dic.begin();
         i != dic.end(); i++) {
        int tmp = i->second; //(i->first).c_str,
        fwrite(&(i->second), sizeof(int), 1, fp_dic);
        int siz = (i->first).size();
        fwrite(&siz, sizeof(int), 1, fp_dic);
        fwrite((i->first).c_str(), sizeof(char), (i->first).size(), fp_dic);
    }
    fclose(fp_dic);

	double time2 = timestamp();
    cout << "SplitTotal = " << splitTotal << "; splitNum = " << splitNum << endl;
	cout << "Compressing time(s) : " << time2 - time1 << endl;
	 
	// to calculate the compression ratio
	stat(dic_file, &s);
	size_t dic_length = s.st_size;
	stat(output_file, &s);
	size_t output_length = s.st_size;
	stat(blockoff_path.c_str(), &s);
	size_t block_length = s.st_size;
	stat(rule_path.c_str(), &s);
	size_t rules_length = s.st_size;

	output_length += dic_length; 
	cout << "Orginal size : " << original_length << " " << ", Compressed size : " << output_length << ", Dictionary size : " << dic_length << ", Block size : " << block_length << ", Rules size : " << rules_length << endl;
	cout << "Compression ratio (text+dic) : " << (double)original_length / (output_length + dic_length) << endl;
    cout << "Compression ratio (text+dic+block) : " << (double)original_length / (output_length + dic_length + block_length) << endl;
    cout << "Dictionary occ : " << (double)dic_length / (output_length + dic_length + block_length) << endl;
    cout << "Block occ : " << (double)block_length / (output_length + dic_length + block_length) << endl;
    
    cout << "Compression ratio (text+dic+rules) : " << (double)original_length / (output_length + dic_length + rules_length) << endl;
    cout << "Compression ratio (text+dic+rules+block) : " << (double)original_length / (output_length + dic_length + block_length + rules_length) << endl;
    cout << "Dictionary occ : " << (double)dic_length / (output_length + dic_length + block_length + rules_length) << endl;
    cout << "Rules occ : " << (double)rules_length / (output_length + dic_length + block_length + rules_length) << endl;
    cout << "Block occ : " << (double)block_length / (output_length + dic_length + block_length + rules_length) << endl;
    cout << endl;
    return 0;
}
