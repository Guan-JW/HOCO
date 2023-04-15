#include "basics.h"
#include <ctime>
#include <signal.h>
#include <stdlib.h>

#define L 3
#define BOUNDARY 5

ulong* split_words;
ulong* split_locations;
ulong split_number;
unordered_map<string, int> *seqMap;

vector<string> dic;
vector<Ele> input;

bool print = false;

void inner_count(int id, int file_index) {
    int length = input[id].length;
    if (length < L) return;
    int word = input[id].id;
    string seq = dic[word];
    for (int i = 1; i < L; i ++) 
        seq += "|" + dic[word];
    seqMap[file_index][seq] += length - L + 1;
}

void inter_count(int id, int file_index) {
    int times = input[id].length < L ? input[id].length : L - 1;

    int total_length = times;
    int end_id = id;
    for(int i = id+1; i < input.size(); i++) {
        total_length += input[i].length;
        if (total_length >= L){
            end_id = i;
            break;
        }
    }
    if (total_length < L)   return;
    
    int cnt_id;
    while (times) {
        cnt_id = id;
        int word = input[cnt_id].id;
        string seq = dic[word];
        int i = 1;
        for (; i < times; i++) seq += "|" + dic[word];
        while (i < L) {
            cnt_id ++;
            if (cnt_id >= input.size()) return;
            word = input[cnt_id].id;
            for (int j = 0; j < min(input[cnt_id].length, L-i); j++) {
                seq += "|" + dic[word];
                i++;
            }
        }
        seqMap[file_index][seq] ++;
        times --;
    }
}

void sequence_count() {
    int i = 0;
	int split_cur = 0;
    for (; i < input.size(); i++) {
        if (input[i].length == 0)
            continue;
        inner_count(i, split_cur);

        if (input[i].id == split_words[split_cur] && split_cur < split_number) {
            split_cur ++;
            continue;
        }
        inter_count(i, split_cur);
    }
}

int main(int argc, char **argv) {

    clock_t t1, t2, t3;
    t1 = clock();
    initialize(argv[1], dic);
    t2 = clock();

    seqMap = new unordered_map<string, int>[split_number];
    sequence_count();

    t3 = clock();
	cout << "IO(s): " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl;
    cout << "Computation(s): " << (double)(t3 - t2) / CLOCKS_PER_SEC << endl;
    cout << "Total(s): " << (double)(t3 - t1) / CLOCKS_PER_SEC << endl;

    return 0;
}