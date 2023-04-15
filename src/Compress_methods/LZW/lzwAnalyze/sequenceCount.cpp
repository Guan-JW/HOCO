#include "basics.h"
#include <ctime>

#define L 3
#define BOUNDARY 5

ulong* split_words;
ulong* split_locations;
ulong split_number;
vector<TremFreq> freq_sort;

unordered_map<string, int> *seqMap;
vector<string> dic;
vector<rule> rules;

string past1;
string past2;
string past3;
int cur_index = 0;

void make_sequence(int word, int file_index) {
    static int file_cur = 0;

	if(file_index == file_cur)
        cur_index ++;
	else{
		cur_index = 0;
        file_cur = file_index;
		past1 = "";
        past2 = "";
        past3 = "";
    }

    // to get word;
    past1 = past2;
    past2 = past3;
	past3 = dic[word - DICMAXSIZE];

	if(cur_index >= 3) {
		string sequence = past1 + "|" + past2 + "|" + past3;
        seqMap[file_index][sequence]++;
    }
    return;
}

void sequence_count(int word, int file_index) {
    if (word < DICMAXSIZE) { // if is rule
        sequence_count(rules[word].p, file_index);
        make_sequence(rules[word].c, file_index);
    } else { // if is word
        make_sequence(word, file_index);
    }
}

int getC0(int word) {
    if (word >= DICMAXSIZE)
        return word;
    else
        return getC0(rules[word].p);
}

int main(int argc, char **argv) {
	clock_t t1, t2, t3;
	t1 = clock();

    LzwInfo info = init(argv[1],dic);

    t2 = clock();

    rules.resize(info.ruleSize);
    seqMap = new unordered_map<string, int>[split_number];
    int *p = info.data;
    int ruleID = 0, code = p[0];
    rule r{};

    r.p = code;
	int split_cur = 0;
    sequence_count(code, split_cur);

    for (int i = 1; i < info.totalSize; i++) {
        code = p[i];
        if (code < DICMAXSIZE && code >= ruleID) {
            r.c = getC0(r.p);
        } else
            r.c = getC0(code);

        rules[ruleID++] = r;

        sequence_count(code, split_cur);


        r.p = code;
		
		if(code - DICMAXSIZE == split_words[split_cur]){
			
            split_cur++;
        }
    }


	t3 = clock();
	cout << "IO(s): " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl;
    cout << "Computation(s): " << (double)(t3 - t2) / CLOCKS_PER_SEC << endl;
    cout << "Total(s): " << (double)(t3 - t1) / CLOCKS_PER_SEC << endl;

}
