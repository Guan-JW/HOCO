
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

int insert(int searchFile, ulong searchStart_charoff, string str) {

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

    int range = abs(prime_eleoff - eleoff);

    // do insertion
    int word = rule_full[searchFile][newMid+1][eleoff].id; 
    int word_length = dic[word].length();
    local_search_charoff -= rule_full[searchFile][newMid+1][eleoff].local_char_offset;
    ulong run_length = get_run_length(searchFile, newMid, eleoff);   // length of current run
    int inside_run_id = local_search_charoff / word_length;
    local_search_charoff -= inside_run_id * word_length;
    
    // insert to current word
    int sub_length = word_length - local_search_charoff;
    string symbol = dic[word];
    symbol = symbol.insert(local_search_charoff, str);  // a new word
    // see it as a new word, don't find, change it later!!
    int dicSize = dic.size();
    dic[dicSize] = symbol;
    
    Ele tmp;    // create new element for the new word
    tmp.id = dicSize;
    tmp.local_char_offset = inside_run_id * word_length;
    ulong hash_numerator_add = str.length() * (eleoff + 1) + word_length;
    eleoff ++;
    rule_full[searchFile][newMid+1].insert(rule_full[searchFile][newMid+1].begin() + eleoff, tmp);

    int insert_length = str.length();
    if (++inside_run_id < run_length) { // post part of the element
        tmp.id = word;
        tmp.local_char_offset += word_length + insert_length;
        hash_numerator_add += (run_length - inside_run_id) * word_length * 2;
        eleoff ++;
        rule_full[searchFile][newMid+1].insert(rule_full[searchFile][newMid+1].begin() + eleoff, tmp);
    }

    eleoff ++;
    int ele_size = rule_full[searchFile][newMid+1].size();
    if (eleoff < ele_size) {    // need to update elements
        hash_numerator_add += (local_block_size - rule_full[searchFile][newMid+1][ele_size-1].local_char_offset) * 2;
        for (int i = ele_size-1; i > eleoff; i--) {
            hash_numerator_add += (rule_full[searchFile][newMid+1][i].local_char_offset - rule_full[searchFile][newMid+1][i-1].local_char_offset) * 2;
            rule_full[searchFile][newMid+1][i].local_char_offset += insert_length;
        }
        rule_full[searchFile][newMid+1][eleoff].local_char_offset += insert_length;
    }
    for (; newMid + 1 < block_off[searchFile].size(); newMid++) {
        block_off[searchFile][newMid+1].char_offset += insert_length;
        block_off[searchFile][newMid+1].ele_offset += 2;
        block_off[searchFile][newMid+1].hash_numerator += hash_numerator_add;
    }
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
    string str( 64, 'c' );
    int j=0;
    for (; j < query_size; j++) {
        if (j % 100 == 0)
        file_index = rand() % split_number;
        query_file_indexes[j] = file_index;
        fileLength = block_off[file_index][block_off[file_index].size() - 1].char_offset;
        query_insert_offsets[j] = rand() % (fileLength);
    }

    int cnt = 0;
    clock_t time8, time9;
    time8 = clock();
    for (int i = 0; i < query_size; i++) {
        insert(query_file_indexes[i], query_insert_offsets[i], str);
        cnt++;
    }
    time9 = clock();
    cout << "IO(s) : " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl;
    cout << "INSERT TIME : " << (double)(time9 - time8) / CLOCKS_PER_SEC << "s" << endl;
    cout << "AVGLatency(s): " << (double)(time9 - time8) / CLOCKS_PER_SEC / query_size << endl;
    cout << "AVGLatency(us): " << (double)(time9 - time8) / CLOCKS_PER_SEC / query_size * 1000000 << endl;
    cout << "Throughput(op/s): " << (double)query_size * CLOCKS_PER_SEC / (time9 - time8) << endl;

    free(query_file_indexes);
    free(query_insert_offsets);
    return 0;
}