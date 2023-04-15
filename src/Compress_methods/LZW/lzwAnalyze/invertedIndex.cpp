#include "basics.h"
#include <ctime>

ulong* split_words;
ulong* split_locations;
ulong split_number;

unordered_map<string, vector<int>> dicMap;
vector<string> dic;

vector<rule> rules;
vector<int> word_temp;
vector<int> counter;

int getC0(int word) {
    if (word >= DICMAXSIZE)
        return word;
    else
        return getC0(rules[word].p);
}
void wCount(int word) {
    if (word < DICMAXSIZE) {
        wCount(rules[word].p);
        int cur = rules[word].c - DICMAXSIZE;
        if (counter[cur] == 0) {
            word_temp.push_back(cur);
        }
        counter[cur]++;
    } else {
        int cur = word - DICMAXSIZE;
		if(counter[cur] == 0){
            word_temp.push_back(cur);
        }
        counter[cur]++;
    }
}

int main(int argc, char **argv) {
	clock_t t1, t2, t3;
    t1 = clock();
	LzwInfo info = init(argv[1], dic);
   t2 = clock();

	rules.resize(info.ruleSize); // initialize dictionary size
    counter.resize(info.dicSize); // res
    counter.assign(info.dicSize, 0);
	
	int *p = info.data;
    int ruleID = 0;
    int code = p[0];
    rule r{};

	r.p = code;
	int split_cur = 0;

    for (int i = 1; i < info.totalSize; i ++){
	    code = p[i];
	    if(code < DICMAXSIZE && code >= ruleID){
			r.c = getC0(r.p);
		}
		else r.c = getC0(r.p);

		rules[ruleID++] = r;
        wCount(code);

        if(code - DICMAXSIZE == split_words[split_cur]){
			for(auto it : word_temp){
               dicMap[dic[it]].push_back(split_cur);
            }
            split_cur++;
            word_temp.clear();
        }
    }

	t3 = clock();
	cout << "IO(s): " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl;
    cout << "Computation(s): " << (double)(t3 - t2) / CLOCKS_PER_SEC << endl;
    cout << "Total(s): " << (double)(t3 - t1) / CLOCKS_PER_SEC << endl;
}
