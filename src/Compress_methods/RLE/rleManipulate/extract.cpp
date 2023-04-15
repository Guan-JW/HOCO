
#include "basics.h"
#include <ctime>

map<int, string> dic;
vector<int> input;
ulong split_number;
ulong* split_locations;
ulong* split_words;

vector<vector<BlockOffset>> block_off;
vector<vector<vector<Ele>>> rule_full;

int get_run_length(int searchFile, int newMid, int eleoff) {
    ulong run_length = 1;
    if (eleoff + 1 < rule_full[searchFile][newMid+1].size())
        run_length = rule_full[searchFile][newMid+1][eleoff+1].local_char_offset - rule_full[searchFile][newMid+1][eleoff].local_char_offset;
    else if (newMid >= 0)
        run_length = block_off[searchFile][newMid+1].char_offset -  block_off[searchFile][newMid].char_offset - rule_full[searchFile][newMid+1][eleoff].local_char_offset;
    else
        run_length = block_off[searchFile][newMid+1].char_offset - rule_full[searchFile][newMid+1][eleoff].local_char_offset;
    return run_length;
}

int extract(int searchFile, ulong searchStart_charoff, int extract_length) {
    int elength = extract_length;
    if (searchStart_charoff >= block_off[searchFile][block_off[searchFile].size()-1].char_offset) {
        cout << "WRONG! Range exceeded!\n";
        cout << searchFile << " " << searchStart_charoff << " " << block_off[searchFile][block_off[searchFile].size()-1].char_offset << endl;
        return -1;
    }
    // block binary search
    int newMid = -1;
    int eleoff;
    ulong local_block_size = block_off[searchFile][0].char_offset;
    ulong local_search_charoff = searchStart_charoff;
    ulong denominator = 1;

    if (local_search_charoff >= local_block_size && block_off[searchFile].size() > 1) {
        int newHead = 0;
        int newEnd = block_off[searchFile].size() - 1;
        newMid = (newHead + newEnd) / 2;
        while (newHead <= newEnd && block_off[searchFile][newMid].char_offset > local_search_charoff ||
            block_off[searchFile][newMid+1].char_offset <= local_search_charoff) {
            if (block_off[searchFile][newHead].char_offset == block_off[searchFile][newMid].char_offset)
                break;
            int oldHead = newHead;
            int oldMid = newMid;
            int oldEnd = newEnd;
            if (local_search_charoff < block_off[searchFile][newMid].char_offset) {
                newEnd = oldMid - 1;
            } else {
                newHead = newMid;
            }
            newMid = (newHead + newEnd) / 2;
        }
        while (newMid + 1 < block_off[searchFile].size() && block_off[searchFile][newMid+1].char_offset <= local_search_charoff)
            newMid ++;
        if (newMid+1 >= block_off[searchFile].size())  newMid = block_off[searchFile].size() - 2;
        local_search_charoff = local_search_charoff - block_off[searchFile][newMid].char_offset;
        local_block_size = block_off[searchFile][newMid+1].char_offset - block_off[searchFile][newMid].char_offset;
    }

    // local hash offset mapping
    denominator = (local_block_size - 1) * local_block_size / 2;
    if (denominator)
        eleoff = local_search_charoff * block_off[searchFile][newMid+1].hash_numerator / denominator;
    else
        eleoff = 0;
    int prime_eleoff = eleoff;

    if (eleoff >= rule_full[searchFile][newMid+1].size())
        eleoff = rule_full[searchFile][newMid+1].size() - 1;
    else if (eleoff < 0)
        eleoff = 0;
    while (rule_full[searchFile][newMid+1][eleoff].local_char_offset > local_search_charoff) {
        eleoff --;
    }
    while (eleoff + 1 < rule_full[searchFile][newMid+1].size() && rule_full[searchFile][newMid+1][eleoff+1].local_char_offset <= local_search_charoff)
        eleoff ++;

    // do extraction
    string result = "";
    int word = rule_full[searchFile][newMid+1][eleoff].id; 
    int word_length = dic[word].length();
    local_search_charoff -= rule_full[searchFile][newMid+1][eleoff].local_char_offset;
    ulong run_length = get_run_length(searchFile, newMid, eleoff);   // length of current run
    int inside_run_id = local_search_charoff / word_length;
    local_search_charoff -= inside_run_id * word_length;

    // deal with current word
    if (word_length - local_search_charoff >= extract_length){  // start point within this word, return
        result += dic[word].substr(local_search_charoff, extract_length);
        return 1;
    }
    // continue extracting
    int sub_length = word_length - local_search_charoff;
    result += dic[word].substr(local_search_charoff, sub_length);
    inside_run_id ++; // next id inside the run 
    if (inside_run_id >= run_length) {  // this run over
        eleoff ++;
        if (eleoff >= rule_full[searchFile][newMid+1].size()) {
            eleoff = 0;
            newMid ++;
            if (newMid >= block_off[searchFile].size()) {   // end of the file, error condition
                return 0;
            }
        }
        run_length = get_run_length(searchFile, newMid, eleoff);
        inside_run_id = 0;
    }

    extract_length -= local_search_charoff;
    string symbol = dic[word];
    while (extract_length > 0) {
        while(inside_run_id < run_length) {
            result += symbol;
            extract_length -= word_length;
            inside_run_id ++;
        }
        eleoff ++;
        if (eleoff >= rule_full[searchFile][newMid+1].size()) {
            eleoff = 0;
            newMid ++;
            if (newMid >= block_off[searchFile].size()) return 0;   // error condition
        }
        if (extract_length <= 0)    break;
        run_length = get_run_length(searchFile, newMid, eleoff);
        word = rule_full[searchFile][newMid+1][eleoff].id;
        symbol = dic[word];
        word_length = symbol.length();
        inside_run_id = 0;
    }
    result = result.substr(0, elength);
    return 1;
}

int main(int argc, char **argv) {
    clock_t t1, t2, t3;
    t1 = clock();
    initialize(argv[1], block_off, rule_full);
    t2 = clock();

    int query_size = 100;
    int query_malloc_size = query_size * sizeof(int);
    int *query_file_indexes = (int *)malloc(query_malloc_size);
    ulong *query_insert_offsets = (ulong *)malloc(query_size*sizeof(ulong));
    
    ulong fileLength;
    int file_index = 0;
    int searchLen=64;
    int j=0;
    for (; j < query_size; j++) {
        file_index = rand() % split_number;
        query_file_indexes[j] = file_index;
        fileLength = block_off[file_index][block_off[file_index].size() - 1].char_offset;
        if (fileLength <= searchLen) {
            j--;
            continue;
        }
        query_insert_offsets[j] = rand() % (fileLength - searchLen);
    }

    int cnt = 0;
    clock_t time8, time9;
    time8 = clock();
    for (int i = 0; i < query_size; i++) {
        extract(query_file_indexes[i], query_insert_offsets[i], searchLen);
        cnt++;
    }
    time9 = clock();
    cout << "IO(s) : " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl;
    cout << "EXTRACT TIME : " << (double)(time9 - time8) / CLOCKS_PER_SEC << "s" << endl;
    cout << "AVGLatency(s): " << (double)(time9 - time8) / CLOCKS_PER_SEC / query_size << endl;
    cout << "AVGLatency(us): " << (double)(time9 - time8) / CLOCKS_PER_SEC / query_size * 1000000 << endl;
    cout << "Throughput(op/s): " << (double)query_size * CLOCKS_PER_SEC / (time9 - time8) << endl;

    free(query_file_indexes);
    free(query_insert_offsets);
    return 0;
}