
#include "basics.h"
#include <ctime>

vector<string> dic;
vector<Ele> input;
unordered_map<string, vector<int>> iiDic;

ulong* split_words;
ulong* split_locations;
ulong split_number;

int main(int argc, char **argv) {
    clock_t t1, t2, t3;
    t1 = clock();
    initialize(argv[1], dic);
    t2 = clock();
    
    ulong split_cur = 0;
    map<string, int> word_temp;
    for (int i = 0; i < input.size(); i ++) {
        int x = input[i].id;
        if (split_cur < split_number && x == split_words[split_cur]) {
            for( auto it : word_temp ) {
                if (iiDic.find(it.first) == iiDic.end()) {
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
            string word = dic[x];
            if (word_temp.find(word) == word_temp.end()) {
                ulong size = word_temp.size();
                word_temp[word] = size;
            }
        }
    }
    

    t3 = clock();
	cout << "IO(s): " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl;
    cout << "Computation(s): " << (double)(t3 - t2) / CLOCKS_PER_SEC << endl;
    cout << "Total(s): " << (double)(t3 - t1) / CLOCKS_PER_SEC << endl;

    return 0;    
}