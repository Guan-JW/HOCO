#include "basics.h"
#include <ctime>


ulong* split_words;
ulong* split_locations;
ulong split_number;

vector<int> counter;
vector<TremFreq> freq_sort;
vector<string> dic;
vector<rule> rules; // p. c

void wCount(int word) {
    if (word < DICMAXSIZE) { // if is rule
        wCount(rules[word].p); // count past
        counter[rules[word].c - DICMAXSIZE]++; // count c
    } else { 
        counter[word - DICMAXSIZE]++;
    }
}

int getC0(int word) {
    if (word >= DICMAXSIZE) // if is word
        return word;
    else
        return getC0(rules[word].p); // return the referred rule / ok, c is the current word.
}

int main(int argc, char **argv) {
	clock_t t1, t2, t3, t4, t5, t6;
    t1 = clock();
    LzwInfo info = init(argv[1], dic);

    rules.resize(info.ruleSize); // initialize dictionary size
    counter.resize(info.dicSize); // res
    counter.assign(info.dicSize, 0);
    int *p = info.data;  // input buffer
    int ruleID = 0, code = p[0];
    rule r{}; // temp rule

	t2 = clock();
    r.p = code;

    for (int i = 1; i < info.totalSize; i++) {  
        code = p[i]; // count code
        if (code < DICMAXSIZE && code >= ruleID) { // if not found 
            r.c = getC0(r.p); // c = old[0]
        } else // if found
            r.c = getC0(code); // c = dic[0]
        rules[ruleID++] = r; // record rule

        wCount(code); // count for code  // count for each c and the first p

        r.p = code; // old
    }

   
	t3 = clock();
	cout << "IO(s): " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl;
    cout << "Computation(s): " << (double)(t3 - t2) / CLOCKS_PER_SEC << endl;
    cout << "Total(s): " << (double)(t3 - t1) / CLOCKS_PER_SEC << endl;

}
