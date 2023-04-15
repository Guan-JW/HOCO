
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

map<string, int> dic;
vector<int> input;
vector<int> counter;

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

    char relationDir[100];
    sprintf(relationDir,"%s",argv[1]);

    FILE *fp;
    if ((fp = fopen(relationDir, "r")) == NULL) {
        cout << "wrong" << endl;
        return 0;
    }

    string myStr;
    int no = 0;
    while ((myStr = getWord(fp)).size() > 0) {
        int x;
        if (dic.find(myStr) == dic.end()) {
            x = no;
            dic[myStr] = no;
            no ++;
        }
        else
            x = dic[myStr];
        input.push_back(x);
    }
    counter.resize(no);

    t2 = clock();
    for (int i = 0; i < input.size(); i ++) {
        counter[input[i]] ++;
    }
    t3 = clock();

	cout << "IO(s): " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl;
    cout << "Computation(s): " << (double)(t3 - t2) / CLOCKS_PER_SEC << endl;
    cout << "Total(s): " << (double)(t3 - t1) / CLOCKS_PER_SEC << endl;

    return 0;
}