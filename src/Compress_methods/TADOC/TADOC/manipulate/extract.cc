#include"common.h"
#include<ctime>
#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<cstring>
#include<vector>
#include<map>
#include<set>
#include<unordered_map>
#include<string>
#include<list>
#include<sys/time.h>
#include<stack>
#include<algorithm>

#include<unordered_set>

#include<fstream>
#include <sstream>
#include <fcntl.h>    // for O_BINARY
#include <unistd.h>
#include <sys/times.h>
#include <limits.h>

using namespace std;

struct BlockOffset {
    ulong char_offset;
    ulong ele_offset;   // if larger than 32767 then bootstrap
    ulong hash_numerator;
    long rfid;
};

struct Ele {
    long id;
    ulong local_char_offset;
    Ele(){id=0; local_char_offset=0;}
};

// DAG
unordered_map<ulong,string> dictionary_use; // word id - string 
vector<vector<vector<Ele>>> rule_full;  //  file/rule - block - element

long words, rules;
long origin_rules;
int splitNum;
long dic_space = 100000000;
int cnt = 0;
ulong* rule_char_length;    // character length of each rule except for the root rule
vector<vector<BlockOffset>> block_off;  // each rule holds an block offset array

// file
int *splitLocation;
set<int> splitSet;

// insert
vector<string> insert_strings;

int rule_extract(int searchFile, int rid, ulong extract_length, vector<string> & rsts) {
  if (extract_length <= 0) return 0;

  string rst_word;
  int cnt_block_off_id;
  if (rid)  cnt_block_off_id = splitNum+rid;
  else  cnt_block_off_id = searchFile;

  int size = block_off[cnt_block_off_id].size();
  for (int newMid = -1; newMid < size - 1; newMid ++) {
    int direct_id = block_off[cnt_block_off_id][newMid+1].rfid <= splitNum ? 0 : block_off[cnt_block_off_id][newMid+1].rfid - splitNum;
    int block_id;
    int rf_id;
    if (direct_id >= origin_rules){ rf_id = splitNum+direct_id; block_id = 0;}
    else if (direct_id == 0) { rf_id = searchFile; block_id = newMid+1; }
    else {rf_id = splitNum + direct_id; block_id = newMid+1; }
    
    for (vector<Ele>::iterator it=rule_full[rf_id][block_id].begin(); it!=rule_full[rf_id][block_id].end(); it++) {
      int ele = it->id;
      if (ele < words) {
        rst_word = dictionary_use[ele];
        if (extract_length <= rst_word.length()) {
          rsts.push_back(rst_word.substr(0, extract_length));
          return 0;
        }
        rsts.push_back(rst_word);
        extract_length -= rst_word.length();
      }
      else {
        extract_length = rule_extract(searchFile, ele - words, extract_length, rsts);
        if(!extract_length) return 0;
      }
    }
  }
  return extract_length;
}

void initialization_basic(char* argv, unordered_map<ulong,string> & dictionary_use, long & words, long & rules){
  char dictionaryDir[100];
  char inputDir[100];
  char offsetPath[100];
  char block_off_Path[100];
  char relationDir[100];
  sprintf(dictionaryDir,"%s/dictionary.dic",argv);
  sprintf(inputDir,"%s/rowCol.dic",argv);
  sprintf(offsetPath,"%s/offset.txt",argv);
  sprintf(block_off_Path,"%s/block_offset.txt",argv);
  sprintf(relationDir, "%s/fileYyNO.txt", argv);

// file split position
  ifstream frelation(relationDir);
  frelation >> splitNum;
  int *split = new int[splitNum];
  memset(split, 0, splitNum);
  splitLocation = new int[splitNum];
  memset(splitLocation, 0, splitNum);
  int tmp;
  for (int i = 0; i < splitNum; i++) {
    frelation >> tmp >> split[i];
  }
  frelation.close();

// build dictionary
  ifstream fin(dictionaryDir);
  ulong tem_num;
  string tem_word;
  while(!fin.eof()){
    tem_num=-1;
    fin>>tem_num;
    fin.get();
    tem_word=getWordDictionary(fin);
    dictionary_use[tem_num]=tem_word;
    if (tem_word == string(" ") || tem_word == string("\t") ||
        tem_word == string("\n")) {
        splitSet.insert(tem_num);
    }
  }
  fin.close();

// load graph and offset
  ifstream finput(inputDir);
  ifstream offsetinput(offsetPath);
  ifstream blockoff_input(block_off_Path);
  int fileid; 
  finput>>fileid;
  finput>>words>>rules;
  origin_rules = rules;

  long ruleSize = 0;
  long cnt_ruleSize = 0;
  for(long i=0; i<rules + splitNum; i++) { // load block offset
    string line;
    vector<BlockOffset> bo;
    getline(blockoff_input, line);
    istringstream ss(line);
    ulong char_offset;
    ulong ele_offset;
    ulong old_ele_offset = 0;
    ulong hash_numerator;
    if(i == 0 || i > splitNum) {
      finput >> ruleSize;
      cnt_ruleSize = 0;
    }
    vector<vector<Ele>> rule_block_full;  
    while(ss >> char_offset >> ele_offset >> hash_numerator) {
      BlockOffset tempt;
      tempt.char_offset = char_offset;
      tempt.ele_offset = ele_offset;
      tempt.hash_numerator = hash_numerator;
      tempt.rfid = i;
      bo.push_back(tempt);
      vector<Ele> block_full;
      for(ulong j=old_ele_offset; j<ele_offset; j++) {
        Ele tempt;
        finput >> tempt.id;
        tempt.id = tempt.id >= words ? tempt.id + dic_space : tempt.id;
        offsetinput >> tempt.local_char_offset;
        block_full.push_back(tempt);
      }
      cnt_ruleSize += ele_offset - old_ele_offset;
      rule_block_full.push_back(block_full);
      old_ele_offset = ele_offset;
    }
    rule_full.push_back(rule_block_full);
    block_off.push_back(bo);
  }
  finput.close();
  offsetinput.close();
  blockoff_input.close();

  words += dic_space;
}


// searchStart_charoff: local character offset within a file
ulong forward_map(int searchFile, ulong searchStart_charoff, ulong extract_length, stack<int> &rule_stack, 
                          stack<ulong> &eleoff_stack, stack<int> &block_stack, vector<string> &rsts) {
// binary search for locating block
// root rule search
  int eleoff;
  int newMid = -1;
  int ele_startoff = searchFile == 0 ? 0 : splitLocation[searchFile-1];
  ulong local_block_size = block_off[searchFile][0].char_offset;
  ulong local_search_charoff = searchStart_charoff;
  ulong denominator = 1;
  if (searchStart_charoff >= block_off[searchFile][0].char_offset && block_off[searchFile].size() > 1){  // skip the first block
    int newHead = 0;
    int newEnd = block_off[searchFile].size() - 1;
    newMid = (newHead + newEnd) / 2;
    while (newHead <= newEnd && block_off[searchFile][newMid].char_offset > searchStart_charoff ||
          block_off[searchFile][newMid+1].char_offset <= searchStart_charoff){   // block offset binary search 
      if (block_off[searchFile][newHead].char_offset == block_off[searchFile][newMid].char_offset) 
        break;
      int oldHead = newHead;
      int oldMid = newMid;
      int oldEnd = newEnd;
      if (searchStart_charoff < block_off[searchFile][newMid].char_offset) {
          newEnd = oldMid - 1;
      } else {
          newHead = oldMid;
      }
      newMid = (newHead + newEnd) / 2;
    }
    while (newMid+1 < block_off[searchFile].size() && block_off[searchFile][newMid+1].char_offset <= searchStart_charoff)
      newMid ++;
    if(newMid+1 >= block_off[searchFile].size()) newMid = block_off[searchFile].size() - 2;
    local_search_charoff = searchStart_charoff - block_off[searchFile][newMid].char_offset;
    local_block_size = block_off[searchFile][newMid+1].char_offset - block_off[searchFile][newMid].char_offset;
  }

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
  while (rule_full[searchFile][newMid+1][eleoff].local_char_offset > local_search_charoff) {  // or binary search
    eleoff --;
  }
  while (eleoff + 1 < rule_full[searchFile][newMid+1].size() && rule_full[searchFile][newMid+1][eleoff+1].local_char_offset <= local_search_charoff)
    eleoff ++;
  
  eleoff_stack.push(eleoff);
  block_stack.push(newMid);
  local_search_charoff -= rule_full[searchFile][newMid+1][eleoff].local_char_offset;
  
  long ele = rule_full[searchFile][newMid+1][eleoff].id;
  long rid = 0;
  long old_rid = rid;
  int file_blocks_id;
  while (ele >= words) {  // rule
    rid = ele - words;
    rule_stack.push(rid);
    file_blocks_id = splitNum + rid;
    newMid = -1;
    ele_startoff = 0;
    local_block_size = block_off[file_blocks_id][0].char_offset;
    denominator = 1;
    if (local_search_charoff >= block_off[file_blocks_id][0].char_offset && block_off[file_blocks_id].size() > 1) {
      int newHead = 0;
      int newEnd = block_off[file_blocks_id].size() - 1;
      newMid = (newHead + newEnd) / 2;
      while (newHead <= newEnd && block_off[file_blocks_id][newMid].char_offset > local_search_charoff ||
            block_off[file_blocks_id][newMid+1].char_offset <= local_search_charoff){   // block offset binary search 
        if (block_off[file_blocks_id][newHead].char_offset == block_off[file_blocks_id][newMid].char_offset) 
          break;
        int oldHead = newHead;
        int oldMid = newMid;
        int oldEnd = newEnd;
        if (local_search_charoff < block_off[file_blocks_id][newMid].char_offset) {
            newEnd = oldMid - 1;
        } else {
            newHead = oldMid;
        }
        newMid = (newHead + newEnd) / 2;
      }
      while (newMid+1 < block_off[file_blocks_id].size() && block_off[file_blocks_id][newMid+1].char_offset < local_search_charoff){
        newMid ++;
      }
      if(newMid+1 >= block_off[file_blocks_id].size()) newMid = block_off[file_blocks_id].size() - 2;
    
      local_search_charoff = local_search_charoff - block_off[file_blocks_id][newMid].char_offset;
      local_block_size = block_off[file_blocks_id][newMid+1].char_offset - block_off[file_blocks_id][newMid].char_offset;
    }

    old_rid = rid;
    rid = block_off[file_blocks_id][newMid+1].rfid <= splitNum ? 0 : block_off[file_blocks_id][newMid+1].rfid - splitNum;  // redirect to the rule where elements exist 
    
    denominator = (local_block_size - 1) * local_block_size / 2;
    if (denominator)
      eleoff = local_search_charoff * block_off[file_blocks_id][newMid+1].hash_numerator / denominator;
    else
      eleoff = 0;
    int peleoff = eleoff;

    if (rid >= origin_rules) {
      if (eleoff >= rule_full[splitNum+rid][0].size())
        eleoff = rule_full[splitNum+rid][0].size() - 1;
      else if (eleoff < 0)
        eleoff = 0;
      while (rule_full[splitNum+rid][0][eleoff].local_char_offset > local_search_charoff) {  // or binary search
        eleoff --;
      }
      while (eleoff + 1 < rule_full[splitNum+rid][0].size() && rule_full[splitNum+rid][0][eleoff+1].local_char_offset <= local_search_charoff)
        eleoff ++;
      
      
      ele = rule_full[splitNum+rid][0][eleoff].id;
      local_search_charoff -= rule_full[splitNum+rid][0][eleoff].local_char_offset;
    }
    else {
      if (eleoff >= rule_full[splitNum+rid][newMid+1].size())
        eleoff = rule_full[splitNum+rid][newMid+1].size() - 1;
      else if (eleoff < 0)
        eleoff = 0;
      while (rule_full[splitNum+rid][newMid+1][eleoff].local_char_offset > local_search_charoff) {  // or binary search
        eleoff --;
      }
      while (eleoff + 1 < rule_full[splitNum+rid][newMid+1].size() && rule_full[splitNum+rid][newMid+1][eleoff+1].local_char_offset <= local_search_charoff)
        eleoff ++;
      
      ele = rule_full[splitNum+rid][newMid+1][eleoff].id;
      local_search_charoff -= rule_full[splitNum+rid][newMid+1][eleoff].local_char_offset;
    }
    
    eleoff_stack.push(eleoff);
    block_stack.push(newMid);
  }

// now ele is a word
// start extracting
  string rst_word;
  rst_word = dictionary_use[ele];
  if (local_search_charoff > 0)  // extract from inside the word
    rst_word = rst_word.substr(local_search_charoff);
  if (extract_length <= rst_word.length()) {
    rsts.push_back(rst_word.substr(0, extract_length));
    return 0;
  }
  rsts.push_back(rst_word);
  extract_length -= rst_word.length();

  // continue traverse
  int current_rule_full_file;
  int current_rule_full_block;
  if (rid == 0) {current_rule_full_file = searchFile; current_rule_full_block = newMid+1;}
  else if (rid >= origin_rules) {current_rule_full_file = splitNum+rid; current_rule_full_block = 0;}
  else  {current_rule_full_file = splitNum+rid; current_rule_full_block = newMid+1;}
  
  while (++eleoff < rule_full[current_rule_full_file][current_rule_full_block].size()) { // traverse inside the block
    ele = rule_full[current_rule_full_file][current_rule_full_block][eleoff].id;
    if (ele < words) {  // ele is a word
      rst_word = dictionary_use[ele];
      if (extract_length <= rst_word.length()) {
        rsts.push_back(rst_word.substr(0, extract_length));
        return 0;
      }
      rsts.push_back(rst_word);
      extract_length -= rst_word.length();
    }
    else {  // rule dig
      extract_length = rule_extract(searchFile, ele - words, extract_length, rsts);
      if (!extract_length)  return 0;
    }
  }

  rule_stack.pop();
  eleoff_stack.pop();
  block_stack.pop();
  rid = old_rid;  // go to blocks who follow the leaf block
  if (rid == 0) file_blocks_id = searchFile;
  if (newMid+2 < block_off[file_blocks_id].size()){  
    newMid ++;
    eleoff = 0;
  }
  else if (!rule_stack.empty()) {
    rid = rule_stack.top();
    eleoff = eleoff_stack.top() + 1;  // start from the next element, this one already executed
    newMid = block_stack.top();
    rule_stack.pop();
    eleoff_stack.pop();
    block_stack.pop();
  }
  else  
    return extract_length;

  int old_eleoff = eleoff;
  while (extract_length && rule_stack.size()>=0) {
    if (rid == 0) file_blocks_id = searchFile;
    else  file_blocks_id = splitNum + rid;
    int size = block_off[file_blocks_id].size();
    for (int nm = newMid; nm < size-1; nm++) {  // for each block after newMid
      int direct_id = block_off[file_blocks_id][nm+1].rfid <= splitNum ? 0 : block_off[file_blocks_id][nm+1].rfid - splitNum;
      if (direct_id == 0) {current_rule_full_file = searchFile; current_rule_full_block = nm+1;}
      else if (direct_id >= origin_rules) {current_rule_full_file = splitNum+direct_id; current_rule_full_block = 0;}
      else  {current_rule_full_file = splitNum+direct_id; current_rule_full_block = nm+1;}
      int rule_full_size = rule_full[current_rule_full_file][current_rule_full_block].size();
      for (; eleoff < rule_full_size; eleoff++) { // block newMid+1 start from eleoff
        ele = rule_full[current_rule_full_file][current_rule_full_block][eleoff].id;
        if (ele < words) {
          rst_word = dictionary_use[ele];
          if (extract_length <= rst_word.length()) {
            rsts.push_back(rst_word.substr(0, extract_length));
            return 0;
          }
          rsts.push_back(rst_word);
          extract_length -= rst_word.length();
        }
        else {
          extract_length = rule_extract(searchFile, ele - words, extract_length, rsts);
          if (!extract_length)  return 0;
        }
      }
      eleoff = 0;
    }
    if (!rid) break;
    rid = rule_stack.top();
    eleoff = eleoff_stack.top() + 1;
    old_eleoff = eleoff;
    newMid = block_stack.top();
    rule_stack.pop();
    eleoff_stack.pop();
    block_stack.pop();
  }

  return extract_length;
}


int extract(int searchFile, ulong searchStart_charoff, ulong extract_length) {
  int Rule0Start_eleoff = (searchFile == 0) ? 0 : splitLocation[searchFile - 1];
  int Rule0End_eleoff = splitLocation[searchFile];

  if(searchStart_charoff >= block_off[searchFile][block_off[searchFile].size()-1].char_offset) {
    cout << "WRONG! Range exceeded!\n";
    cout << searchFile << " " << searchStart_charoff << " " << block_off[searchFile][block_off[searchFile].size()-1].char_offset << endl;
    return -1;
  }
  stack<int> rule_stack;
  stack<ulong> eleoff_stack;
  stack<int> block_stack;
  vector<string> rsts;
  rule_stack.push(0); // add the root rule

  ulong remain = forward_map(searchFile, searchStart_charoff, extract_length, rule_stack, 
                              eleoff_stack, block_stack, rsts);
  if (remain) {
    cout << "ERROR. Not enough values for extracting.." << remain << endl;
    return 1;
  }
  return 0;
}

int main(int argc, char **argv) {
  clock_t t1, t2, t3;
  t1 = clock();
  initialization_basic(argv[1], dictionary_use, words, rules);
  t2 = clock();

  int query_size = 100;
  int query_malloc_size = query_size * sizeof(int);
  int *query_file_indexes = (int *)malloc(query_malloc_size);
  ulong *query_insert_offsets = (ulong *)malloc(query_size*sizeof(ulong));

  ulong fileLength;
  int file_index = 0;
  ulong searchLen=64;
  int j=0;
  for(; j<query_size; j++){
    if (j % 100 == 0)
      file_index = rand() % splitNum;
    query_file_indexes[j] = file_index;
    fileLength = block_off[file_index][block_off[file_index].size() - 1].char_offset;
    if (fileLength < searchLen) {
      j--;
      continue;
    }
    query_insert_offsets[j] = rand() % (fileLength - searchLen);
  }

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