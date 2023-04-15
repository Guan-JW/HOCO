
#include "basics.h"
#include <ctime>

vector<string> dic;
vector<Ele> input;
vector<int> counter;
ulong* split_words;
ulong* split_locations;
ulong split_number;

void wordCount () {
    for(int i = 0; i < input.size(); i ++) {
        counter[input[i].id] += input[i].length;
    }
}

int main(int argc, char **argv) {
    clock_t t1, t2, t3;
    t1 = clock();
    initialize(argv[1], dic);
    t2 = clock();

    counter.resize(dic.size());

    wordCount();

    t3 = clock();
	cout << "IO(s): " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl;
    cout << "Computation(s): " << (double)(t3 - t2) / CLOCKS_PER_SEC << endl;
    cout << "Total(s): " << (double)(t3 - t1) / CLOCKS_PER_SEC << endl;
    cout << dic.size() << endl;
    return 0;    
}