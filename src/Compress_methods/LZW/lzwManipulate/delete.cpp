
#include "basics.h"
#include <ctime>
#include <stack>

ulong* split_words;
ulong* split_locations;
ulong split_number;
vector<string> dic;
vector<vector<BlockOffset>> block_off;
vector<vector<vector<Ele>>> rule_full;
vector<rule> rules; // p. c
map<vector<int>, int> rules_rev;

#define PRINT

int Query_delete(int searchFile, ulong searchStart_charoff, ulong delete_length) {

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
    ulong local_search_charoff = searchStart_charoff + delete_length;
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

    if (eleoff >= rule_full[searchFile][newMid+1].size())
        eleoff = rule_full[searchFile][newMid+1].size() - 1;
    else if (eleoff < 0)
        eleoff = 0;

    while (rule_full[searchFile][newMid+1][eleoff].local_char_offset > local_search_charoff) {
        eleoff --;
    }
    while (eleoff + 1 < rule_full[searchFile][newMid+1].size() && rule_full[searchFile][newMid+1][eleoff+1].local_char_offset <= local_search_charoff)
        eleoff ++;

    ulong end_offset = 0;
    if (eleoff + 1 < rule_full[searchFile][newMid+1].size())
        end_offset = rule_full[searchFile][newMid+1][eleoff+1].local_char_offset;
    else if (newMid >= 0)
        end_offset = block_off[searchFile][newMid+1].char_offset - block_off[searchFile][newMid].char_offset;
    else    
        end_offset = block_off[searchFile][newMid+1].char_offset;

    int start_newMid = newMid;
    vector<Ele> remain_rule_full;
    remain_rule_full.assign(rule_full[searchFile][newMid+1].begin()+eleoff, rule_full[searchFile][newMid+1].end()); // start from eleoff, to ease the calculation of hash numerator
    int eleid = rule_full[searchFile][newMid+1][eleoff].id;
    stack<int> wordlist;
    int ele_minus = 0;  // how many elements are being merged
    int block_minus = 0;    // how many blocks are being merged

    vector<int> tmp(2); // for recording prefix
    tmp[0] = -1;
    tmp[1] = -1;
    bool mark_prev = false;
    int prev_char_length = 0;
    while (end_offset > local_search_charoff) {
        if (eleid >= DICMAXSIZE) {  /* word */
            end_offset -= dic[eleid - DICMAXSIZE].length();
            wordlist.push(eleid);
            if (--eleoff >= 0)
                eleid = rule_full[searchFile][newMid+1][eleoff].id;
            else if (--newMid >= -1) {
                eleoff = rule_full[searchFile][newMid+1].size()-1;
                eleid = rule_full[searchFile][newMid+1][eleoff].id;
                block_minus ++;
            }
            break;
        }
        else { /* entry */
            end_offset -= dic[rules[eleid].c-DICMAXSIZE].length();  /* the first word in the entry */
            if (end_offset < local_search_charoff)  mark_prev = true;
            wordlist.push(rules[eleid].c);
            eleid = rules[eleid].p;
        }
    }

    if (end_offset < local_search_charoff) {    // end position within the last word, then cut it
    
        int word = wordlist.top();
        wordlist.pop();
        string oldWord = dic[word - DICMAXSIZE];
        int length = local_search_charoff - end_offset;
        string newWord = oldWord.substr(length, oldWord.length() - length);

        if (length > delete_length) {   //  start positiion within the last word, cut it again
            string preWord = oldWord.substr(0, length - delete_length);
            newWord = preWord + newWord;
            if (mark_prev){     // has prefix
                tmp[0] = eleid; // mark prefix
                prev_char_length = eleoff >= 1 ? end_offset - rule_full[searchFile][newMid+1][eleoff-1].local_char_offset : end_offset;
                wordlist.push(dic.size() + DICMAXSIZE);
            }
            else{    // doesn't has prefix, then use this newWord as prefix, don't push
                tmp[0] = dic.size() + DICMAXSIZE;
                prev_char_length = dic[dic.size()-1].length();
            }
        }
        else
            wordlist.push(dic.size() + DICMAXSIZE);
        dic.push_back(newWord);
        delete_length -= length;
    }
    if (delete_length > 0) {    // need to delete other elements
        ele_minus = 1;
        while (newMid >= -1) {

            while (eleoff >= 0) {
                int word;
                int ele_minus_char_length = 0;
                while (delete_length > 0) {
                    if (eleid >= DICMAXSIZE) {
                        word = eleid - DICMAXSIZE;
                        delete_length -= dic[word].length();
                        break;
                    }
                    else {
                        word = rules[eleid].c - DICMAXSIZE;
                        delete_length -= dic[word].length();
                        ele_minus_char_length += dic[word].length();
                        eleid = rules[eleid].p;
                        if (delete_length < 0) {    // start position within this word
                            tmp[0] = eleid; // prefix
                            if (eleoff + 1 < rule_full[searchFile][newMid+1].size())
                                prev_char_length = rule_full[searchFile][newMid+1][eleoff+1].local_char_offset - rule_full[searchFile][newMid+1][eleoff].local_char_offset - ele_minus_char_length;
                            else if (newMid >= 0)
                                prev_char_length = block_off[searchFile][newMid+1].char_offset - block_off[searchFile][newMid].char_offset - rule_full[searchFile][newMid+1][eleoff].local_char_offset - ele_minus_char_length;
                            else
                                prev_char_length = block_off[searchFile][newMid+1].char_offset - rule_full[searchFile][newMid+1][eleoff].local_char_offset - ele_minus_char_length;
                        }
                    }
                    if (delete_length < 0) {    // start position within this word, cut it
                        string newWord = dic[word];
                        newWord = newWord.substr(0, 0 - delete_length);
                        wordlist.push(dic.size() + DICMAXSIZE);
                        dic.push_back(newWord);
                    }
                }
                if (delete_length <= 0)   break;
                eleoff --;
                if (eleoff >= 0)
                    eleid = rule_full[searchFile][newMid+1][eleoff].id;
                ele_minus ++;
            }
            if (delete_length <= 0)  break;
            newMid --;
            if (newMid+1 >= 0){
                eleoff = rule_full[searchFile][newMid+1].size() - 1;
                eleid = rule_full[searchFile][newMid+1][eleoff].id;
            }
            block_minus ++;
        }
        if (tmp[0] < 0) {  // doesn't have a prefix
            int word = wordlist.top();
            wordlist.pop();
            tmp[0] = word;
            prev_char_length = dic[word - DICMAXSIZE].length();
        }
        if (newMid < -1) {
            newMid = -1;
            eleoff = 0;
        }
    }

    // Do integratioin
    ulong local_hash_numerator = 0;
    for (int i = 1; i <= eleoff; i++) {
        local_hash_numerator += (rule_full[searchFile][newMid+1][i].local_char_offset - 
                                    rule_full[searchFile][newMid+1][i-1].local_char_offset) * (i-1);
    }
    // delete 
    block_off[searchFile].erase(block_off[searchFile].begin() + newMid + 1, block_off[searchFile].begin() + newMid + 1 + block_minus);
    rule_full[searchFile].erase(rule_full[searchFile].begin() + newMid + 2, rule_full[searchFile].begin() + newMid + 2 + block_minus);
    rule_full[searchFile][newMid+1].erase(rule_full[searchFile][newMid+1].begin() + eleoff, rule_full[searchFile][newMid+1].end());

    ulong local_char_offset;
    int total_ele_num;
    if (eleoff < 0) {
        local_char_offset = 0;
        total_ele_num = 0;
    }
    else if (newMid >= -1) {
        local_char_offset = eleoff - 1 >= 0 ? rule_full[searchFile][newMid+1][eleoff-1].local_char_offset : 0;
        if (newMid >= 0)    total_ele_num = block_off[searchFile][newMid].ele_offset + eleoff - 1;
        else    total_ele_num = eleoff - 1;
    }
    else {
        local_char_offset = 0;
        total_ele_num = eleoff - 1;
    }
    
    vector<Ele> new_rule_full;
    while (!wordlist.empty()) { // integrate 
        tmp[1] = wordlist.top();
        wordlist.pop();
        if (rules_rev.find(tmp) == rules_rev.end()) {   // not found
            rules_rev[tmp] = rules.size();
            rules.push_back({tmp[0], tmp[1]});
            local_char_offset += prev_char_length;
            local_hash_numerator += prev_char_length * rule_full[searchFile][newMid+1].size();

            Ele t;
            t.id = tmp[0];
            t.local_char_offset = local_char_offset;
            new_rule_full.push_back(t);
            tmp[0] = tmp[1];
            total_ele_num ++;
            prev_char_length = dic[tmp[1] - DICMAXSIZE].length();
        }
        else{
            prev_char_length += dic[tmp[1] - DICMAXSIZE].length();
            tmp[0] = rules_rev[tmp];
            if (wordlist.empty()) {
                local_char_offset += prev_char_length;
                local_hash_numerator += prev_char_length * rule_full[searchFile][newMid+1].size();

                Ele t;
                t.id = tmp[0];
                t.local_char_offset = local_char_offset;
                new_rule_full.push_back(t);
                tmp[0] = tmp[1];
                total_ele_num ++;
            }
        }
    }
    if (rule_full[searchFile][newMid+1].size() == 0)
        rule_full[searchFile].push_back(new_rule_full);
    else
        rule_full[searchFile][newMid+1].insert(rule_full[searchFile][newMid+1].end(), new_rule_full.begin(), new_rule_full.end());

    total_ele_num += remain_rule_full.size() - 1;
    for (int i = 1; i < remain_rule_full.size(); i++) {
        ulong size = remain_rule_full[i].local_char_offset - remain_rule_full[i-1].local_char_offset;
        local_hash_numerator += size * rule_full[searchFile][newMid+1].size();
        local_char_offset += size;
    }
    if (rule_full[searchFile][newMid+1].size() == 0){
        rule_full[searchFile].push_back(remain_rule_full);
    }
    else
        rule_full[searchFile][newMid+1].insert(rule_full[searchFile][newMid+1].end(), remain_rule_full.begin(), remain_rule_full.end());

    if (block_off[searchFile].size() == 0) {    // create new block
        BlockOffset bo;
        bo.char_offset = local_char_offset;
        bo.hash_numerator = local_hash_numerator;
        bo.ele_offset = total_ele_num;
        bo.rfid = searchFile;
        block_off[searchFile].push_back(bo);
    }
    else {
        ulong old_char_offset = block_off[searchFile][newMid+1].char_offset;
        int old_ele_offset = block_off[searchFile][newMid+1].ele_offset;
        block_off[searchFile][newMid+1].char_offset = local_char_offset;
        block_off[searchFile][newMid+1].hash_numerator = local_hash_numerator;
        block_off[searchFile][newMid+1].ele_offset = total_ele_num;
        
        for (int i = newMid+2; i < block_off[searchFile].size(); i++) {
            local_char_offset += block_off[searchFile][i].char_offset - old_char_offset;
            total_ele_num += block_off[searchFile][i].ele_offset - old_ele_offset;
            old_char_offset = block_off[searchFile][i].char_offset;
            old_ele_offset = block_off[searchFile][i].ele_offset;
            block_off[searchFile][i].char_offset = local_char_offset;
            block_off[searchFile][i].ele_offset = total_ele_num;
        }
    }
    return 1;
}

int main(int argc, char **argv) {
    clock_t t1, t2, t3;
    t1 = clock();
    LzwInfo info = init(argv[1], dic, block_off, rule_full);
    get_rules_rev(rules_rev);
	t2 = clock();

    int query_size = 100;
    int query_malloc_size = query_size * sizeof(int);
    int *query_file_indexes = (int *)malloc(query_malloc_size);
    ulong *query_delete_offsets = (ulong *)malloc(query_size*sizeof(ulong));
    
    ulong fileLength;
    int file_index = 0;
    ulong deleteLen=64;
    int j=0;
    clock_t time8, time9;
    double total_time = 0;
    for (; j < query_size; j++) {
        file_index = rand() % split_number;
        query_file_indexes[j] = file_index;
        fileLength = block_off[file_index][block_off[file_index].size() - 1].char_offset;
        if (fileLength <= deleteLen) {
            continue;
        }
        query_delete_offsets[j] = rand() % (fileLength - deleteLen);
        time8 = clock();
        Query_delete(query_file_indexes[j], query_delete_offsets[j], deleteLen);
        time9 = clock();
        total_time += (double)(time9 - time8);
    }

    cout << "IO(s) : " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl;
    cout << "DELETE TIME : " << (double)total_time / CLOCKS_PER_SEC << "s" << endl;
    cout << "AVGLatency(s): " << (double)total_time / CLOCKS_PER_SEC / query_size << endl;
    cout << "AVGLatency(us): " << (double)total_time / CLOCKS_PER_SEC / query_size * 1000000 << endl;
    cout << "Throughput(op/s): " << (double)query_size * CLOCKS_PER_SEC / total_time << endl;

    free(query_file_indexes);
    free(query_delete_offsets);
    return 0;
}