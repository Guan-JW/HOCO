
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

#include "sys/time.h"
using namespace std;

map<string, int> dic;
map<int, string> dic_rev;
vector<int> input;
int no;
int splitNum;
int *splitLocation;

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
    double time1 = timestamp();
    if(argc <= 2){
		cout << "please specify the input file path." << endl;
		return 0;
	}

	char dic_file[500];
    char output_file[500];
	sprintf(dic_file, "%s/dictionary.txt", argv[2]);
	sprintf(output_file, "%s/file.bin", argv[2]);
    FILE *fp_dic;
    fp_dic = fopen(dic_file, "wb"); // create a file as dictionary
    FILE *fp_file;
    fp_file = fopen(output_file, "wb"); // create a file as output

    string blockoff_path = argv[2];
    blockoff_path += "/block_offset.txt";
    ofstream fblock(blockoff_path);

    string offset_path = argv[2];
    offset_path += "/offset.txt";
    ofstream foffset(offset_path);

    FILE *fp;
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
    int tmp;
    for (int i = 0; i < splitNum; i++) {
        frelation >> tmp >> split[i];
    }
    frelation.close();


    string myStr;
    int n = 0;
    no = 0;
    int curtmpSize = 0, ruleID = 0;
    int inputSize = 0;
    // here just to make a dictionary
    while ((myStr = getWord(fp)).size() > 0) { // haven't meet the end
        int x;                    // hash the char or string
        if (dic.find(myStr) == dic.end()) {    // if the char doesn't exist
            x = no;
            dic[myStr] = no;
            dic_rev[no] = myStr;
            no++;
        }
        else    
            x = dic[myStr];
        input.push_back(x);
    }
    inputSize = input.size();


    int splitCur = 0;

    int c = input[0];
    int old_c = c;
    int run_size = 1;
    int block_size = 4096 / sizeof(int);
    int block_charoffset = 0;
    int block_hash_numerator = 0;
    int block_eleoff = 0;
    int total_eleoff = 0;
    int cnt_eleoff = 0;
    int local_charoffset = 0;
    foffset << local_charoffset << " ";
    int splitTotal = 0;
    int i = 1;
    for (; i < inputSize; i++) {
        c = input[i];                 // c is the i^th word (int)
        if (c != old_c) {
            fwrite(&old_c, sizeof(int), 1, fp_file);
            int char_length = run_size * dic_rev[old_c].length();
            local_charoffset += char_length;
            
            
            block_charoffset += char_length;
            block_hash_numerator += char_length * cnt_eleoff;
            cnt_eleoff ++;
            if (c == split[splitCur] && splitCur < splitNum) {
                block_eleoff += cnt_eleoff;
                fblock << block_charoffset << " " << block_eleoff << " " << block_hash_numerator << " ";
                total_eleoff += block_eleoff;
                fblock << endl;

                block_hash_numerator = 0;
                block_charoffset = 0;
                block_eleoff = 0;
                cnt_eleoff = 0;
                local_charoffset = 0;

                splitLocation[splitCur] = block_eleoff + cnt_eleoff;
                splitTotal ++;
                splitCur++;
                if (++i < inputSize)
                    c = input[i];   // put next word
            }
            else if (cnt_eleoff == block_size) {
                block_eleoff += cnt_eleoff;
                fblock << block_charoffset << " " << block_eleoff << " " << block_hash_numerator << " ";
                block_hash_numerator = 0;
                cnt_eleoff = 0;
                local_charoffset = 0;
            }
            foffset << local_charoffset << " "; // write offset
            run_size = 1;
            old_c = c;
        }
        run_size ++;
    }
    if (cnt_eleoff) {   // remain elements, create a block
        fwrite(&old_c, sizeof(int), 1, fp_file);
        int char_length = run_size * dic_rev[old_c].length();
        local_charoffset += char_length;
        foffset << local_charoffset; // write offset
            
        block_charoffset += char_length;
        block_hash_numerator += char_length * cnt_eleoff;
        cnt_eleoff ++;
        block_eleoff += cnt_eleoff;
        fblock << block_charoffset << " " << block_eleoff << " " << block_hash_numerator;
        total_eleoff += block_eleoff;
    }
    double time2 = timestamp();
	cout << "Compressing time(s) : " << time2 - time1 << endl;
    fblock.close();
    fclose(fp_file);

    int dicSize = dic.size();
    fwrite(&(dicSize), sizeof(int), 1, fp_dic);
    fwrite(&(total_eleoff), sizeof(int), 1, fp_dic);
    for (map<string, int>::iterator i = dic.begin();
         i != dic.end(); i++) {
        int tmp = i->second; //(i->first).c_str,
        fwrite(&(i->second), sizeof(int), 1, fp_dic);
        int siz = (i->first).size();
        fwrite(&siz, sizeof(int), 1, fp_dic);
        fwrite((i->first).c_str(), sizeof(char), (i->first).size(), fp_dic);
    }
    fclose(fp_dic);


    // to calculate the compression ratio
	stat(dic_file, &s);
	size_t dic_length = s.st_size;
	stat(output_file, &s);
	size_t output_length = s.st_size;
	stat(blockoff_path.c_str(), &s);
	size_t block_length = s.st_size;

	cout << "Orginal size : " << original_length << " " << ", Compressed size : " << output_length << ", Dictionary size : " << dic_length << ", Block size : " << block_length << endl;
	cout << "Compression ratio (text+dic) : " << (double)original_length / (output_length + dic_length) << endl;
    cout << "Compression ratio (text+dic+block) : " << (double)original_length / (output_length + dic_length + block_length) << endl;
    cout << "Dictionary occ : " << (double)dic_length / (output_length + dic_length + block_length) << endl;
    cout << "Block occ : " << (double)block_length / (output_length + dic_length + block_length) << endl;
    
    return 0;
}