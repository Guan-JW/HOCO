// #define _GNU_SOURCE

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <sys/stat.h> 
#include <unistd.h>

#include <iostream>
#include <unordered_map>
#include <vector>

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
vector<int> input;
int no;

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
	// string file_path = argv[2];

    FILE *fp_dic;
	char dic_file[500];
	char output_file[500];
	sprintf(dic_file, "%s/dictionary.txt", argv[2]);
	sprintf(output_file, "%s/file.bin", argv[2]);

    fp_dic = fopen(dic_file, "wb"); // create a file as dictionary

    FILE *fp_file;
    fp_file = fopen(output_file, "wb"); // create a file as output

    FILE *fp;
    char word[99999];

    char ruletmp[99999];

    unordered_map<vector<int>, int, VectorHasher>
        rules; // hashmap from a vector to a interger
    unordered_map<int, string>
        rules2word; // hashmap from a interger to a string

    if ((fp = fopen(argv[1], "r")) == NULL)
        cout << "wrong" << endl;
    // if ((fp = fopen("19_NSR.txt", "r")) == NULL) cout << "wrong" << endl;
	struct stat s{};
	stat(argv[1], &s);
	size_t original_length = s.st_size;

    string myStr;

    int outputSize = 0; // Size of nums in the output file

    int n = 0;
    char c;
    char *ptr = word;
    no = DICMAXSIZE;

    vector<int> tmp(2);
    dic[string(1, ' ')] = no;
    tmp[0] = no;
    tmp[1] = no;
    rules[tmp] = no++;

    dic[string(1, '\t')] = no;
    tmp[0] = no;
    tmp[1] = no;
    rules[tmp] = no++;

    dic[string(1, '\n')] = no;
    tmp[0] = no;
    tmp[1] = no;
    rules[tmp] = no++;

    dic[string(1, '\r')] = no;
    tmp[0] = no;
    tmp[1] = no;
    rules[tmp] = no++;

    dic[string(1, ',')] = no;
    tmp[0] = no;
    tmp[1] = no;
    rules[tmp] = no++;

    dic[string(1, '.')] = no;
    tmp[0] = no;
    tmp[1] = no;
    rules[tmp] = no++;

    int curtmpSize = 0, ruleID = 0;

    int inputSize = 0;

    // here just to make a dictionary
    while ((myStr = getWord(fp)).size() > 0) { // haven't meet the end
        int x = dic[myStr];                    // hash the char or string
        if (x == 0) {                          // if the char doesn't exist
            x = no;
            dic[myStr] = no;
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

    w.push_back(0);

    int ruleSize = 0; // Size of rules

    double time3 = timestamp();

    // Here is the lzw compress
    for (int i = 0; i < inputSize; i++) { // word by word
        int c = input[i];                 // c is the i^th word (int)

        vector<int> wc(w);

        wc.push_back(c);
        wc[0] += c; // make the vector hashable

        if (rules.count(wc))
            // c in dic, add it and come to next iteration
            w = wc;
        else {
            // c not in dic, add wc to dic and write w to file
            fwrite(&rules[w], sizeof(int), 1, fp_file);
            rules[wc] = ruleSize++; // become new words
            w.clear();
            w.push_back(c);
            w.push_back(c);
            outputSize++;
        }
    }
    // if end with a word in dictionary
    if (!w.empty()) {
        fwrite(&rules[w], sizeof(int), 1, fp_file);
        outputSize++;
    }


    // printf("time (s): %lf\n", time2 - time1);
    // printf("first part time (s): %lf\n", time3 - time1);
    fclose(fp);
    fclose(fp_file);
    // exit(0);
    // int ruleSize = rules.size();
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

	cout << "Compressing time(s) : " << time2 - time1 << endl;
	 
	// to calculate the compression ratio
	stat(dic_file, &s);
	size_t dic_length = s.st_size;
	stat(output_file, &s);
	size_t output_length = s.st_size;

	output_length += dic_length; 
	cout << "orginal size : " << original_length << " " << ", compression size : " << output_length << endl;
	cout << "Compression ratio : " << (double)100 * output_length / original_length << " %" << endl;

    return 0;
}
