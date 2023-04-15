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
unordered_map<string,ulong> dictionary_reverse; // string - word id
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
    dictionary_reverse[tem_word]=tem_num; // reverse dictionary
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
ulong forward_map(int searchFile, ulong searchStart_charoff, stack<int> &rule_stack, 
                          stack<ulong> &eleoff_stack, stack<int> &block_stack, 
                          vector<BlockOffset> &root_file_blocks, vector<Ele> &newWords, string &str) {
// binary search for locating block
// root rule search
  int eleoff;
  int newMid = -1;
  int ele_startoff = searchFile == 0 ? 0 : splitLocation[searchFile-1];
  ulong local_block_size = root_file_blocks[0].char_offset;
  ulong local_search_charoff = searchStart_charoff;
  ulong denominator = 1;
  if (searchStart_charoff >= root_file_blocks[0].char_offset && root_file_blocks.size() > 1){  // skip the first block
    int newHead = 0;
    int newEnd = root_file_blocks.size() - 1;
    newMid = (newHead + newEnd) / 2;
    while (newHead <= newEnd && root_file_blocks[newMid].char_offset > searchStart_charoff ||
          root_file_blocks[newMid+1].char_offset <= searchStart_charoff){   // block offset binary search 
      if (root_file_blocks[newHead].char_offset == root_file_blocks[newMid].char_offset) 
        break;
      int oldHead = newHead;
      int oldMid = newMid;
      int oldEnd = newEnd;
      if (searchStart_charoff < root_file_blocks[newMid].char_offset) {
          newEnd = oldMid - 1;
      } else {
          newHead = oldMid;
      }
      newMid = (newHead + newEnd) / 2;
    }
    while (newMid+1 < root_file_blocks.size() && root_file_blocks[newMid+1].char_offset <= searchStart_charoff)
      newMid ++;
    if(newMid+1 >= root_file_blocks.size()) newMid = root_file_blocks.size() - 2;
    local_search_charoff = searchStart_charoff - root_file_blocks[newMid].char_offset;
    local_block_size = root_file_blocks[newMid+1].char_offset - root_file_blocks[newMid].char_offset;
  }

  denominator = (local_block_size - 1) * local_block_size / 2;
  if (denominator)
    eleoff = local_search_charoff * root_file_blocks[newMid+1].hash_numerator / denominator;
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
  vector<BlockOffset> file_blocks;
  while (ele >= words) {  // rule
    rid = ele - words;
    rule_stack.push(rid);
    file_blocks = block_off[splitNum + rid];
    newMid = -1;
    ele_startoff = 0;
    local_block_size = file_blocks[0].char_offset;
    denominator = 1;
    if (local_search_charoff >= file_blocks[0].char_offset && file_blocks.size() > 1) {
      
      int newHead = 0;
      int newEnd = file_blocks.size() - 1;
      newMid = (newHead + newEnd) / 2;
      while (newHead <= newEnd && file_blocks[newMid].char_offset > local_search_charoff ||
            file_blocks[newMid+1].char_offset <= local_search_charoff){   // block offset binary search 
        if (file_blocks[newHead].char_offset == file_blocks[newMid].char_offset) 
          break;
        int oldHead = newHead;
        int oldMid = newMid;
        int oldEnd = newEnd;
        if (local_search_charoff < file_blocks[newMid].char_offset) {
            newEnd = oldMid - 1;
        } else {
            newHead = oldMid;
        }
        newMid = (newHead + newEnd) / 2;
      }
      while (newMid+1 < file_blocks.size() && file_blocks[newMid+1].char_offset < local_search_charoff){
        newMid ++;
      }
      if(newMid+1 >= file_blocks.size()) newMid = file_blocks.size() - 2;
    
      local_search_charoff = local_search_charoff - file_blocks[newMid].char_offset;
      local_block_size = file_blocks[newMid+1].char_offset - file_blocks[newMid].char_offset;
    }

    old_rid = rid;
    rid = file_blocks[newMid+1].rfid <= splitNum ? 0 : file_blocks[newMid+1].rfid - splitNum;  // redirect to the rule where elements exist 
    
    denominator = (local_block_size - 1) * local_block_size / 2;
    if (denominator)
      eleoff = local_search_charoff * file_blocks[newMid+1].hash_numerator / denominator;
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
  string newWord = str;
  ulong local_char_move = newWord.length();
  if (local_search_charoff > 0) { // insert into the word
    newWord = dictionary_use[ele];
    newWord.insert(local_search_charoff, str);
  }
  
  // update dictionary
  if (dictionary_reverse.find(newWord) == dictionary_reverse.end()) { 
    ulong dic_size = dictionary_use.size();
    dictionary_use[dic_size] = newWord;
    dictionary_reverse[newWord] = dic_size;
  }

  Ele tmp;
  tmp.id = dictionary_reverse[newWord];
  // insert into the root rule, update directly
  if(rid == 0) {  
    tmp.local_char_offset = rule_full[searchFile][newMid+1][eleoff].local_char_offset;
    ulong hash_numerator_add = local_char_move * eleoff + block_off[searchFile][newMid+1].char_offset - tmp.local_char_offset;
    // updatae elements
    if (local_search_charoff > 0)
      rule_full[searchFile][newMid+1][eleoff] = tmp;  // replace
    else{
      rule_full[searchFile][newMid+1].insert(rule_full[searchFile][newMid+1].begin()+eleoff, tmp);  // insert
      block_off[searchFile][newMid+1].ele_offset++;
    }
    for(vector<Ele>::iterator it=rule_full[searchFile][newMid+1].begin()+eleoff+1; it!=rule_full[searchFile][newMid+1].end(); it++) {
      it->local_char_offset += local_char_move;
    }
    // update block list
    block_off[searchFile][newMid+1].hash_numerator += hash_numerator_add;
    for(vector<BlockOffset>::iterator it=block_off[searchFile].begin()+newMid+1; it!=block_off[searchFile].end(); it++) {
      it->char_offset += local_char_move;
      if (local_search_charoff == 0)
        it->ele_offset ++;
    }

    return 0;
  }
  // insert into the leaf node, copy block
  else {
    // leaf node
    vector<Ele> block_rule_full;
    if(rid >= origin_rules)
      block_rule_full = rule_full[splitNum+rid][0];
    else
      block_rule_full = rule_full[splitNum+rid][newMid+1];
    tmp.local_char_offset = block_rule_full[eleoff].local_char_offset;
    // create new element block 
    vector<Ele> newRule;
    vector<BlockOffset> new_blockoff;
    ulong hash_numerator_add = local_char_move * eleoff + block_off[splitNum+rid][newMid+1].char_offset - tmp.local_char_offset;

    newRule.assign(block_rule_full.begin(), block_rule_full.begin()+eleoff);
    newRule.push_back(tmp);
    if (local_search_charoff == 0) {
      Ele tempt;
      tempt.id = block_rule_full[eleoff].id;
      tempt.local_char_offset = block_rule_full[eleoff].local_char_offset + local_char_move;
      newRule.push_back(tempt);
    }
    for(vector<Ele>::iterator it=block_rule_full.begin()+eleoff+1; it!=block_rule_full.end(); it++) {
      Ele tempt;
      tempt.id = it->id;
      tempt.local_char_offset = it->local_char_offset + local_char_move;
      newRule.push_back(tempt);
    }
    vector<vector<Ele>> new_rule_block_full;
    new_rule_block_full.push_back(newRule);
    rule_full.push_back(new_rule_block_full);

    rid = old_rid;
    new_blockoff.assign(block_off[splitNum+rid].begin(), block_off[splitNum+rid].begin()+newMid+1); // point to the old rule
    BlockOffset tempt;
    tempt.char_offset = block_off[splitNum+rid][newMid+1].char_offset + local_char_move;
    tempt.ele_offset = newRule.size();
    tempt.hash_numerator = block_off[splitNum+rid][newMid+1].hash_numerator + hash_numerator_add;
    tempt.rfid = splitNum + rules;  // point to the new rule
    new_blockoff.push_back(tempt);
    new_blockoff.insert(new_blockoff.end(), block_off[splitNum+rid].begin()+newMid+2, block_off[splitNum+rid].end()); // old rule
    for(vector<BlockOffset>::iterator it=new_blockoff.begin()+newMid+2; it!=new_blockoff.end(); it++) {
      it->char_offset += local_char_move;
    }
    block_off.push_back(new_blockoff);
    rules ++;
    rule_stack.pop();
    eleoff_stack.pop();
    block_stack.pop();
  
    while(rule_stack.size() > 1) {
      long rid = rule_stack.top();
      ulong eleoff = eleoff_stack.top();
      int newMid = block_stack.top();
      rule_stack.pop();
      eleoff_stack.pop();
      block_stack.pop();

      vector<Ele> newRule;
      vector<BlockOffset> new_blockoff;
      vector<Ele> block_rule_full;
      new_blockoff.assign(block_off[splitNum+rid].begin(), block_off[splitNum+rid].end());
      rid = block_off[splitNum+rid][newMid+1].rfid <= splitNum ? 0 : block_off[splitNum+rid][newMid+1].rfid - splitNum;
      if(rid >= origin_rules) 
        block_rule_full =rule_full[splitNum+rid][0];
      else
        block_rule_full =rule_full[splitNum+rid][newMid+1];

      ulong hash_numerator_add = local_char_move * eleoff;
      // create element block
      newRule.assign(block_rule_full.begin(), block_rule_full.end());
      newRule[eleoff].id = words + rules - 1; // pointer change from old rule to the new rule
      for(vector<Ele>::iterator it=newRule.begin()+eleoff+1; it!=newRule.end(); it++) {
        it->local_char_offset += local_char_move;
      }
      vector<vector<Ele>> new_rule_block_full;
      new_rule_block_full.push_back(newRule);
      rule_full.push_back(new_rule_block_full);
      
      new_blockoff[newMid+1].rfid = splitNum + rules; // point to the new rule
      new_blockoff[newMid+1].hash_numerator += hash_numerator_add;
      for(vector<BlockOffset>::iterator it=new_blockoff.begin()+newMid+1; it!=new_blockoff.end(); it++) {
        it->char_offset += local_char_move;
      }
      block_off.push_back(new_blockoff);
      
      rules ++;
    }

    // root rule
    eleoff = eleoff_stack.top();
    newMid = block_stack.top();
    rule_full[searchFile][newMid+1][eleoff].id = words + rules - 1;
    hash_numerator_add = local_char_move * eleoff;
    for(vector<Ele>::iterator it=rule_full[searchFile][newMid+1].begin()+eleoff+1; it!=rule_full[searchFile][newMid+1].end(); it++) {
      it->local_char_offset += local_char_move;
    }
    block_off[searchFile][newMid+1].hash_numerator += hash_numerator_add;
    for(vector<BlockOffset>::iterator it=block_off[searchFile].begin()+newMid+1; it!=block_off[searchFile].end(); it++) {
      it->char_offset += local_char_move;
    }

  }
  
  return local_char_move; // insert into subrule
}


int insert(int searchFile, ulong searchStart_charoff, string &str) {

  int Rule0Start_eleoff = (searchFile == 0) ? 0 : splitLocation[searchFile - 1];
  int Rule0End_eleoff = splitLocation[searchFile];
  vector<BlockOffset> root_file_blocks = block_off[searchFile];
  if(searchStart_charoff >= root_file_blocks[root_file_blocks.size()-1].char_offset) {
    cout << "WRONG! Range exceeded!\n";
    return -1;
  }

  stack<int> rule_stack;
  stack<ulong> eleoff_stack;
  stack<int> block_stack;
  vector<Ele> newWords;
  rule_stack.push(0); // add the root rule

  ulong local_char_move = forward_map(searchFile, searchStart_charoff, rule_stack, 
                              eleoff_stack, block_stack, root_file_blocks, newWords, str);

  return 1;
}

void get_new_size(double & rule_size, double & block_size ) {
  rule_size = 0;
  block_size = 0;
  for (int i = splitNum + origin_rules; i < rule_full.size(); i++) {
    rule_size += rule_full[i][0].size();
    block_size += block_off[i].size();
  }
  rule_size *= sizeof(int);
  block_size *= sizeof(int) * 4;
}

int main(int argc, char **argv) {
  clock_t t1, t2, t3;
  t1 = clock();
  initialization_basic(argv[1], dictionary_use, words, rules);
  t2 = clock();

  int query_size = 100;
  int query_malloc_size = query_size * sizeof(int);
  int *query_file_indexes = (int *)malloc(query_malloc_size);
  int *query_insert_offsets = (int *)malloc(query_malloc_size);

  int fileLength, file_index;
  string str( 64, 'c' );
  int searchLen=64;
  for(int j=0; j<query_size; j++){
    if (j % 100 == 0)
      file_index = rand() % splitNum;
    query_file_indexes[j] = file_index;
    fileLength = block_off[file_index][block_off[file_index].size() - 1].char_offset;
    query_insert_offsets[j] = rand() % (fileLength - searchLen);
    insert_strings.push_back(str);
  }
  char size_path[100];

  clock_t time8, time9;
  double execute_time = 0;
  double current_time = 0;
  double latency;
  double rule_size, block_size;
  
  for (int i = 0; i < query_size; i++) {
    time8 = clock();
		insert(query_file_indexes[i], query_insert_offsets[i], insert_strings[i]);
    time9 = clock();
    execute_time += time9 - time8;
    current_time += time9 - time8;
    cnt++;
  }
  
  cout << "IO(s) : " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl;
  cout << "INSERT TIME : " << (double)execute_time / CLOCKS_PER_SEC << "s" << endl;
	cout << "AVGLatency(s): " << (double)execute_time / CLOCKS_PER_SEC / query_size << endl;
	cout << "AVGLatency(us): " << (double)execute_time / CLOCKS_PER_SEC / query_size * 1000000 << endl;
	cout << "Throughput(op/s): " << (double)query_size * CLOCKS_PER_SEC / execute_time << endl;

  free(query_file_indexes);
  free(query_insert_offsets);
  return 0;
}