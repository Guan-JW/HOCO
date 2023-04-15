
#include "basics.h"
#include <ctime>

ulong* split_words;
ulong* split_locations;
ulong split_number;
vector<string> dic;
vector<vector<BlockOffset>> block_off;
vector<vector<vector<Ele>>> rule_full;
vector<rule> rules; // p. c

int extract(int searchFile, ulong searchStart_charoff, ulong extract_length) {

    if (searchStart_charoff >= block_off[searchFile][block_off[searchFile].size()-1].char_offset) {
        cout << "WRONG! Range exceeded!\n";
        cout << searchFile << " " << searchStart_charoff << " " << block_off[searchFile][block_off[searchFile].size()-1].char_offset << endl;
        return -1;
    }
    vector<string> rsts;
    // block binary search
    int newMid = -1;
    int eleoff;
    ulong local_block_size = block_off[searchFile][0].char_offset;
    // do searching based on the end position, from back to front 
    ulong local_search_charoff = searchStart_charoff + extract_length;
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
    ulong end_offset = 0;
    if (eleoff + 1 < rule_full[searchFile][newMid+1].size())
        end_offset = rule_full[searchFile][newMid+1][eleoff+1].local_char_offset;
    else if (newMid >= 0)
        end_offset = block_off[searchFile][newMid+1].char_offset - block_off[searchFile][newMid].char_offset;
    else    
        end_offset = block_off[searchFile][newMid+1].char_offset;
    
    int eleid = rule_full[searchFile][newMid+1][eleoff].id;
    string result = "";
    while (end_offset > local_search_charoff) {
        if (eleid >= DICMAXSIZE) {  // if is a word
            end_offset -= dic[eleid - DICMAXSIZE].length();
            result = dic[eleid - DICMAXSIZE].substr(0, local_search_charoff - end_offset) + result;
            if (--eleoff >= 0)
                eleid = rule_full[searchFile][newMid+1][eleoff].id;
            else if (--newMid >= -1) {
                eleoff = rule_full[searchFile][newMid+1].size()-1;
                eleid = rule_full[searchFile][newMid+1][eleoff].id;
            }
            break;
        }
        else {
            end_offset -= dic[rules[eleid].c-DICMAXSIZE].length();
            eleid = rules[eleid].p;
        }
    }
    
    while (newMid >= -1) {
        while (eleoff >= 0) {
            while (result.length() < extract_length) {
                if (eleid >= DICMAXSIZE) {
                    result = dic[eleid - DICMAXSIZE] + result;
                    break;
                }
                else {
                    result = dic[rules[eleid].c - DICMAXSIZE] + result;
                    eleid = rules[eleid].p;
                }
            }
            if (result.length() >= extract_length)   break;
            if (--eleoff >= 0)
                eleid = rule_full[searchFile][newMid+1][eleoff].id;
        }
        if (result.length() >= extract_length)  break;
        newMid --;
        if (--newMid+1 >= 0){
            eleoff = rule_full[searchFile][newMid+1].size() - 1;
            eleid = rule_full[searchFile][newMid+1][eleoff].id;
        }
    }
    
    result = result.substr(result.length()-extract_length, extract_length);
    return 1;
}

int main(int argc, char **argv) {
    clock_t t1, t2, t3;
    t1 = clock();
    LzwInfo info = init(argv[1], dic, block_off, rule_full);
	t2 = clock();

    int query_size = 100;
    
    int query_malloc_size = query_size * sizeof(int);
    int *query_file_indexes = (int *)malloc(query_malloc_size);
    ulong *query_insert_offsets = (ulong *)malloc(query_size*sizeof(ulong));
    
    ulong fileLength;
    int file_index = 0;
    ulong searchLen=64;
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