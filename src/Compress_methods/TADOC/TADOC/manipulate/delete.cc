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
unordered_map<long,string> dictionary_use; // word id - string 
unordered_map<string,long> dictionary_reverse; // string - word id
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

void initialization_basic(char* argv, unordered_map<long,string> & dictionary_use, long & words, long & rules){
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
  long tem_num;
  string tem_word;
  while(!fin.eof()){
    tem_num=-1;
    fin>>tem_num;
    fin.get();
    tem_word=getWordDictionary(fin);
    dictionary_use.insert({tem_num, tem_word});
    dictionary_reverse.insert({tem_word, tem_num});
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


ulong rule_delete_from_start(int searchFile, long rid, ulong delete_length, long & new_rule) {
  
  if (delete_length <= 0) return 0;
  
  vector<BlockOffset> new_block_off;
  vector<BlockOffset> refer_block_off;
  vector<Ele> new_rule_full;
  if (rid == 0){
    new_block_off.assign(block_off[searchFile].begin(), block_off[searchFile].end());
    refer_block_off = block_off[searchFile];
  }
  else{  
    new_block_off.assign(block_off[splitNum+rid].begin(), block_off[splitNum+rid].end());
    refer_block_off = block_off[splitNum+rid];
  }
  int block_num = new_block_off.size();
  int newMid = -1;
  for (; newMid < block_num-1; newMid ++) { // for each block

    long direct_id = refer_block_off[newMid+1].rfid <= splitNum ? 0 : refer_block_off[newMid+1].rfid - splitNum;
    
    vector<Ele> cnt_rule_full;  // locate the block 
    if (direct_id == 0) cnt_rule_full = rule_full[searchFile][newMid+1];
    else if (direct_id >= origin_rules) cnt_rule_full = rule_full[splitNum+direct_id][0];
    else  cnt_rule_full = rule_full[splitNum+direct_id][newMid+1];

    long size = cnt_rule_full.size();

    ulong local_ele_offset = 0;
    ulong local_char_offset = 0;
    ulong hash_numerator = 0;
    long eleoff = 0;
    for (; eleoff < size; eleoff ++) {
      long ele = cnt_rule_full[eleoff].id;
      if (ele < words) {
        string old_word = dictionary_use[ele];
        if (delete_length < old_word.length()) {  // first element
          Ele tmp;
          string new_word = old_word.substr(delete_length);
          if (dictionary_reverse.find(new_word) == dictionary_reverse.end()) {
            long dic_size = dictionary_use.size();
            dictionary_use.insert({dic_size, new_word});
            dictionary_reverse.insert({new_word, dic_size});
          }
          tmp.id = dictionary_reverse[new_word];
          tmp.local_char_offset = local_char_offset;
          hash_numerator += new_word.length() * new_rule_full.size();
          new_rule_full.push_back(tmp); // add word
          local_char_offset += new_word.length();
          local_ele_offset ++;
          delete_length = 0;
        }
        else
          delete_length -= old_word.length();
      }
      else {
        long new_rule = -1;
        ulong new_delete_length = rule_delete_from_start(searchFile, ele - words, delete_length, new_rule);

        if (!new_delete_length && new_rule >= 0) { // rule length larger than delete_length, or else the whole rule is deleted
          Ele tmp;
          tmp.id = new_rule + words;
          tmp.local_char_offset = local_char_offset;
          ulong diff = block_off[splitNum+new_rule][block_off[splitNum+new_rule].size()-1].char_offset;
          local_char_offset += diff;
          hash_numerator += diff * new_rule_full.size();
          new_rule_full.push_back(tmp);
          local_ele_offset ++;
        }
        delete_length = new_delete_length;
      }

      if (delete_length <= 0){
        eleoff ++;
        local_ele_offset += size - eleoff;
        for (; eleoff < size; eleoff ++) {  // insert rest of the block
          Ele tmp;
          tmp.id = cnt_rule_full[eleoff].id;
          tmp.local_char_offset = local_char_offset;
          
          ulong diff = 0;
          if (eleoff + 1 < size)
            diff = cnt_rule_full[eleoff+1].local_char_offset - cnt_rule_full[eleoff].local_char_offset;
          else {
            if (newMid < 0) diff = refer_block_off[newMid+1].char_offset - cnt_rule_full[eleoff].local_char_offset;
            else  diff = refer_block_off[newMid+1].char_offset - refer_block_off[newMid].char_offset - cnt_rule_full[eleoff].local_char_offset;
          }
          local_char_offset += diff;
          hash_numerator += diff * new_rule_full.size();
          new_rule_full.push_back(tmp);
        }

        vector<vector<Ele>> rf;
        rf.push_back(new_rule_full);
        rule_full.push_back(rf);

        new_block_off[newMid+1].rfid = rules + splitNum;
        new_block_off[newMid+1].hash_numerator = hash_numerator;
        if (newMid >= 0) {
          local_ele_offset += new_block_off[newMid].ele_offset;
          local_char_offset += new_block_off[newMid].char_offset;
        }
        new_block_off[newMid+1].ele_offset = local_ele_offset;
        new_block_off[newMid+1].char_offset = local_char_offset;
            
        for (int nm = newMid+2; nm < block_num; nm ++) {
          local_ele_offset += refer_block_off[nm].ele_offset - refer_block_off[nm-1].ele_offset;
          local_char_offset += refer_block_off[nm].char_offset - refer_block_off[nm-1].char_offset;
          new_block_off[nm].ele_offset = local_ele_offset;
          new_block_off[nm].char_offset = local_char_offset;
        }
        block_off.push_back(new_block_off);
        new_rule = rules;
        rules++;
        return 0;
      }

    }

    // if all elements in the block are deleted, then create an empty block
    new_block_off[newMid+1].rfid = rules + splitNum;
    new_block_off[newMid+1].ele_offset = 0; // since delete from start
    new_block_off[newMid+1].char_offset = 0;
    new_block_off[newMid+1].hash_numerator = 0;
    
  }
  // all deleted
  return delete_length;
}

// searchStart_charoff: local character offset within a file
ulong forward_map(int searchFile, ulong searchStart_charoff, ulong delete_length, stack<long> &rule_stack, 
                          stack<ulong> &eleoff_stack, stack<int> &block_stack, 
                          vector<BlockOffset> &root_file_blocks, vector<string> &rsts) {

// binary search for locating block
// root rule search
  long eleoff;
  int newMid = -1;
  long ele_startoff = searchFile == 0 ? 0 : splitLocation[searchFile-1];
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
  
// sub rule search
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
    else  eleoff = 0;
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
  ulong hash_numerator_minus = 0;
  ulong local_char_offset = 0;
  ulong local_char_move = delete_length;
  Ele tmp;
  tmp.id = -1;  // means no nead to replace element
  
  string old_word = dictionary_use[ele];
  if (local_search_charoff > 0) {
    string new_word = old_word.substr(0, local_search_charoff); // first part
    if (delete_length < old_word.substr(local_search_charoff).length()) { // concate
      new_word += old_word.substr(local_search_charoff + delete_length);
    }
    if (dictionary_reverse.find(new_word) == dictionary_reverse.end()) { // update dictionary
      long dic_size = dictionary_use.size();
      dictionary_use.insert({dic_size, new_word});
      dictionary_reverse.insert({new_word, dic_size});
    }
    tmp.id = dictionary_reverse[new_word];
    ulong diff = old_word.length() - new_word.length();
    hash_numerator_minus = diff * eleoff;
    delete_length -= diff;
    local_char_offset += new_word.length();
  }
  

  // update element in the root rule
  // only root can directly update element, since no parents
  if (rid == 0) {
    local_char_offset += rule_full[searchFile][newMid+1][eleoff].local_char_offset;
    if (local_search_charoff > 0) {  // to replace element, no block rearrange
      tmp.local_char_offset = rule_full[searchFile][newMid+1][eleoff].local_char_offset;
      rule_full[searchFile][newMid+1][eleoff] = tmp;  // replace eleoff
      eleoff ++;
      if (delete_length <= 0) { // finish
        for (vector<Ele>::iterator it=rule_full[searchFile][newMid+1].begin()+eleoff; it!=rule_full[searchFile][newMid+1].end(); it++) {
          it->local_char_offset -= local_char_move;
        }
        block_off[searchFile][newMid+1].hash_numerator -= hash_numerator_minus;
        for (vector<BlockOffset>::iterator it=block_off[searchFile].begin()+newMid+1; it!=block_off[searchFile].end(); it++) {
          it->char_offset -= local_char_move;
        }
        return 0;
      }
    }

    // traverse + delete
    vector<Ele> new_rule_full;
    new_rule_full.assign(rule_full[searchFile][newMid+1].begin(), rule_full[searchFile][newMid+1].begin()+eleoff);
   
    ulong hash_numerator = 0;
    ulong local_ele_offset = eleoff;
    for (long i = 1; i < eleoff; i++) {
      hash_numerator += (new_rule_full[i].local_char_offset - new_rule_full[i-1].local_char_offset) * (i-1);
    }
    long size = rule_full[searchFile][newMid+1].size();

    // elements in the block
    for (; eleoff < size; eleoff++) {
      ele = rule_full[searchFile][newMid+1][eleoff].id;
      if (ele < words) {
        old_word = dictionary_use[ele];
        Ele tempt;
        // finish inside the block
        if (delete_length < old_word.length()) {  // finish
          string new_word = old_word.substr(delete_length);
          if (dictionary_reverse.find(new_word) == dictionary_reverse.end()) { // update dictionary
            ulong dic_size = dictionary_use.size();
            dictionary_use.insert({dic_size, new_word});
            dictionary_reverse.insert({new_word, dic_size});
          }
          tempt.id = dictionary_reverse[new_word];
          tempt.local_char_offset = local_char_offset;
          hash_numerator += new_word.length() * new_rule_full.size();
          new_rule_full.push_back(tempt);

          local_char_offset += new_word.length();
          local_ele_offset ++;
          delete_length = 0;
        }
        else 
          delete_length -= old_word.length();
        
      }
      else {
        long new_rule = -1;
        ulong new_delete_length = rule_delete_from_start(searchFile, ele - words, delete_length, new_rule);
        if (!new_delete_length && new_rule >= 0) { // rule length larger than delete_length, or else the whole rule is deleted
          Ele tmp;
          tmp.id = new_rule + words;
          tmp.local_char_offset = local_char_offset;
          ulong diff = block_off[splitNum+new_rule][block_off[splitNum+new_rule].size()-1].char_offset;
          local_char_offset += diff;
          hash_numerator += diff * new_rule_full.size();
          new_rule_full.push_back(tmp);
          local_ele_offset ++;
        }
        delete_length = new_delete_length;
      }

      if (delete_length <= 0) {
        eleoff ++;
        local_ele_offset += size - eleoff;
        // udpate element block
        for (; eleoff < size; eleoff ++) {
          Ele tmp;
          tmp.id = rule_full[searchFile][newMid+1][eleoff].id;
          tmp.local_char_offset = local_char_offset;
          ulong diff = 0;
          if (eleoff + 1 < size)
            diff = rule_full[searchFile][newMid+1][eleoff+1].local_char_offset - rule_full[searchFile][newMid+1][eleoff].local_char_offset;
          else {
            if (newMid < 0) diff = block_off[searchFile][newMid+1].char_offset - rule_full[searchFile][newMid+1][eleoff].local_char_offset;
            else  diff = block_off[searchFile][newMid+1].char_offset - block_off[searchFile][newMid].char_offset - rule_full[searchFile][newMid+1][eleoff].local_char_offset;
          }
          local_char_offset += diff;
          hash_numerator += diff * new_rule_full.size();
          new_rule_full.push_back(tmp);
        }
        rule_full[searchFile][newMid+1] = new_rule_full;

        // update block list
        ulong old_ele_offset = block_off[searchFile][newMid+1].ele_offset;
        ulong old_char_offset = block_off[searchFile][newMid+1].char_offset;
        block_off[searchFile][newMid+1].hash_numerator = hash_numerator;
        if (newMid >= 0) {
          local_ele_offset += block_off[searchFile][newMid].ele_offset;
          local_char_offset += block_off[searchFile][newMid].char_offset;
        }
        block_off[searchFile][newMid+1].ele_offset = local_ele_offset;
        block_off[searchFile][newMid+1].char_offset = local_char_offset;
        for (int nm = newMid + 2; nm < block_off[searchFile].size(); nm ++) {
          local_ele_offset += block_off[searchFile][nm].ele_offset - old_ele_offset;
          local_char_offset += block_off[searchFile][nm].char_offset - old_char_offset;
          old_ele_offset = block_off[searchFile][nm].ele_offset;
          old_char_offset = block_off[searchFile][nm].char_offset;
          block_off[searchFile][nm].ele_offset = local_ele_offset;
          block_off[searchFile][nm].char_offset = local_char_offset;
        }
        return 0;
      }
    }

    // need to merge blocks (delete across multiple blocks)
    int start_newMid = newMid;
    newMid ++;
    // rule_full
    for (; newMid+1 < block_off[searchFile].size(); newMid ++) {
      int direct_id = block_off[searchFile][newMid+1].rfid <= splitNum ? 0 : block_off[searchFile][newMid+1].rfid - splitNum;
      vector<Ele> ref_rule_full;
      if (direct_id == 0) ref_rule_full = rule_full[searchFile][newMid+1];
      else if (direct_id >= origin_rules) ref_rule_full = rule_full[splitNum+direct_id][0];
      else  ref_rule_full = rule_full[splitNum+direct_id][newMid+1];
      int ele_size = ref_rule_full.size();

      for (eleoff = 0; eleoff < ele_size; eleoff ++) {
        int ele = ref_rule_full[eleoff].id;
        if (ele < words) {
          string old_word = dictionary_use[ele];
          if (delete_length < old_word.length()) {
            Ele tmp;
            string new_word = old_word.substr(delete_length);
            if (dictionary_reverse.find(new_word) == dictionary_reverse.end()) {
              long dic_size = dictionary_use.size();
              dictionary_use.insert({dic_size, new_word});
              dictionary_reverse.insert({new_word, dic_size});
            }
            tmp.id = dictionary_reverse[new_word];
            tmp.local_char_offset = local_char_offset;
            hash_numerator += new_word.length();
            new_rule_full.push_back(tmp);
            local_char_offset += new_word.length();
            local_ele_offset ++;
            delete_length = 0;
          }
          else
            delete_length -= old_word.length();

        }
        else {
          long new_rule = -1;
          ulong new_delete_length = rule_delete_from_start(searchFile, ele - words, delete_length, new_rule);
          if (!new_delete_length && new_rule >= 0) {
            Ele tmp;
            tmp.id = new_rule + words;
            tmp.local_char_offset = local_char_offset;
            ulong diff = block_off[splitNum+new_rule][block_off[splitNum+new_rule].size()-1].char_offset;
            local_char_offset += diff;
            hash_numerator += diff * new_rule_full.size();
            new_rule_full.push_back(tmp);
            local_ele_offset ++;
          }
          delete_length = new_delete_length;
        }

        if (delete_length <= 0){  // finish
          eleoff ++;
          local_ele_offset += ele_size - eleoff;
          for (; eleoff < ele_size; eleoff ++ ) {
            Ele tmp;
            tmp.id = ref_rule_full[eleoff].id;
            tmp.local_char_offset = local_char_offset;
            ulong diff = 0;
            if (eleoff + 1 < ele_size)
              diff = ref_rule_full[eleoff+1].local_char_offset - ref_rule_full[eleoff].local_char_offset;
            else {
              if (newMid < 0) diff = block_off[searchFile][newMid+1].char_offset - ref_rule_full[eleoff].local_char_offset;
              else  diff = block_off[searchFile][newMid+1].char_offset - block_off[searchFile][newMid].char_offset - ref_rule_full[eleoff].local_char_offset;
            }
            local_char_offset += diff;
            hash_numerator += diff * new_rule_full.size();
            new_rule_full.push_back(tmp);
          }

          rule_full[searchFile].erase(rule_full[searchFile].begin()+start_newMid+1, rule_full[searchFile].begin()+newMid+1);
          rule_full[searchFile][start_newMid+1] = new_rule_full;

          // root, directly erase
          if (start_newMid >= 0) {
            local_ele_offset += block_off[searchFile][start_newMid].ele_offset;
            local_char_offset += block_off[searchFile][start_newMid].char_offset;
          }
          block_off[searchFile][start_newMid+1].ele_offset = local_ele_offset;
          block_off[searchFile][start_newMid+1].char_offset = local_char_offset;
          block_off[searchFile][start_newMid+1].hash_numerator = hash_numerator;
          for (int nm = newMid+2; nm < block_off[searchFile].size(); nm ++) {
            start_newMid ++;
            local_ele_offset += block_off[searchFile][nm].ele_offset - block_off[searchFile][nm-1].ele_offset;
            local_char_offset += block_off[searchFile][nm].char_offset - block_off[searchFile][nm-1].char_offset;
            block_off[searchFile][start_newMid+1].ele_offset = local_ele_offset;
            block_off[searchFile][start_newMid+1].char_offset = local_char_offset;
          }
          block_off[searchFile].erase(block_off[searchFile].begin()+start_newMid+2, block_off[searchFile].end());
          return 0;
        }
      }
    }
  }

  else {
    long new_rule = -1;
    vector<Ele> ref_rule_full;
    vector<Ele> new_rule_full;
    vector<BlockOffset> new_block_off;
    vector<BlockOffset> ref_block_off;
      
    // do copy
    if (old_rid == 0){
      new_block_off.assign(block_off[searchFile].begin(), block_off[searchFile].end());
      ref_block_off = block_off[searchFile];
    }
    else{
      new_block_off.assign(block_off[splitNum+old_rid].begin(), block_off[splitNum+old_rid].end());
      ref_block_off = block_off[splitNum+old_rid];
    }

    if (rid >= origin_rules) {
      ref_rule_full = rule_full[splitNum+rid][0];
      new_rule_full.assign(rule_full[splitNum+rid][0].begin(), rule_full[splitNum+rid][0].begin()+eleoff);
    }
    else {
      ref_rule_full = rule_full[splitNum+rid][newMid+1];
      new_rule_full.assign(rule_full[splitNum+rid][newMid+1].begin(), rule_full[splitNum+rid][newMid+1].begin()+eleoff);
    }

    local_char_offset += ref_rule_full[eleoff].local_char_offset;

    if (local_search_charoff > 0) {
      tmp.local_char_offset = ref_rule_full[eleoff].local_char_offset;
      new_rule_full.push_back(tmp);
      eleoff ++;
      if (delete_length <= 0) { // finish
        for (vector<Ele>::iterator it=ref_rule_full.begin()+eleoff; it!=ref_rule_full.end(); it++) {
          Ele tempt;
          tempt.id = it->id;
          tempt.local_char_offset = it->local_char_offset - local_char_move;
          new_rule_full.push_back(tempt);
        }
        vector<vector<Ele>> rf;
        rf.push_back(new_rule_full);
        rule_full.push_back(rf);

        new_block_off[newMid+1].hash_numerator -= hash_numerator_minus;
        new_block_off[newMid+1].rfid = rules + splitNum;
        int i = newMid +1;
        for (vector<BlockOffset>::iterator it=new_block_off.begin()+newMid+1; it!=new_block_off.end(); it++) {
          it->char_offset -= local_char_move;
          i++;
        }
        block_off.push_back(new_block_off);
        new_rule = rules; // mark
        rules ++;
      }
    }

    ulong hash_numerator = 0;
    ulong local_ele_offset = eleoff;
    // traverse + delete
    if (new_rule < 0) { // not done
      for (long i = 1; i < eleoff; i ++) {
        hash_numerator += (new_rule_full[i].local_char_offset - new_rule_full[i-1].local_char_offset) * (i-1);
      }
      long size = ref_rule_full.size();
      for (; eleoff < size; eleoff ++) {
        ele = ref_rule_full[eleoff].id;
        if (ele < words) {
          old_word = dictionary_use[ele];
          Ele tempt;
          if (delete_length < old_word.length()) {  // finish
            string new_word = old_word.substr(delete_length);
            if (dictionary_reverse.find(new_word) == dictionary_reverse.end()) {
              long dic_size = dictionary_use.size();
              dictionary_use.insert({dic_size, new_word});
              dictionary_reverse.insert({new_word, dic_size});
            }
            tempt.id = dictionary_reverse[new_word];
            tempt.local_char_offset = local_char_offset;
            hash_numerator += new_word.length() * new_rule_full.size();
            new_rule_full.push_back(tempt);

            local_char_offset += new_word.length();
            local_ele_offset ++;
            delete_length = 0;
          }
          else
            delete_length -= old_word.length();
        }
        else {
          long new_rule = -1;
          ulong new_delete_length = rule_delete_from_start(searchFile, ele - words, delete_length, new_rule);
          if (!new_delete_length && new_rule >= 0) {
            Ele tempt;
            tempt.id = new_rule + words;
            tempt.local_char_offset = local_char_offset;
            ulong diff = block_off[splitNum+new_rule][block_off[splitNum+new_rule].size()-1].char_offset;
            local_char_offset += diff;
            hash_numerator += diff * new_rule_full.size();
            new_rule_full.push_back(tempt);
            local_ele_offset ++;

          }
          delete_length = new_delete_length;
        }
        if (delete_length <= 0) {
          eleoff ++;
          local_ele_offset += size - eleoff;
          for (; eleoff < size; eleoff ++) {
            Ele tempt;
            tempt.id = ref_rule_full[eleoff].id;
            tempt.local_char_offset = local_char_offset;
            ulong diff = 0;
            if (eleoff + 1 < size)  diff = ref_rule_full[eleoff + 1].local_char_offset - ref_rule_full[eleoff].local_char_offset;
            else {
              if (newMid < 0) diff = ref_block_off[newMid+1].char_offset - ref_rule_full[eleoff].local_char_offset;
              else  diff = ref_block_off[newMid+1].char_offset - ref_block_off[newMid].char_offset - ref_rule_full[eleoff].local_char_offset;
            }
            local_char_offset += diff;
            hash_numerator += diff * new_rule_full.size();
            new_rule_full.push_back(tempt);
          }

          vector<vector<Ele>> rf;
          rf.push_back(new_rule_full);
          rule_full.push_back(rf);

          new_block_off[newMid+1].rfid = rules + splitNum;
          new_block_off[newMid+1].hash_numerator = hash_numerator;
          if (newMid >= 0) {
            local_ele_offset += new_block_off[newMid].ele_offset;
            local_char_offset += new_block_off[newMid].char_offset;
          }
          new_block_off[newMid+1].ele_offset = local_ele_offset;
          new_block_off[newMid+1].char_offset = local_char_offset;
            
          for (int nm = newMid + 2; nm < ref_block_off.size(); nm ++) {
            local_ele_offset += ref_block_off[nm].ele_offset - ref_block_off[nm-1].ele_offset;
            local_char_offset += ref_block_off[nm].char_offset - ref_block_off[nm-1].char_offset;
            new_block_off[nm].ele_offset = local_ele_offset;
            new_block_off[nm].char_offset = local_char_offset;
          }
          block_off.push_back(new_block_off);
          new_rule = rules; // mark
          rules ++;
        }
      }
    }

    // need to merge blocks
    if (new_rule < 0) {
      int start_newMid = newMid;
      newMid++;
      for (; newMid + 1 < ref_block_off.size(); newMid ++) {
        long direct_id = ref_block_off[newMid+1].rfid <= splitNum ? 0 : ref_block_off[newMid+1].rfid - splitNum;
        vector<Ele> ref_rule_full;
        if (direct_id == 0) ref_rule_full = rule_full[searchFile][newMid+1];
        else if (direct_id >= origin_rules) ref_rule_full = rule_full[splitNum+direct_id][0];
        else  ref_rule_full = rule_full[splitNum+direct_id][newMid+1];
        long ele_size = ref_rule_full.size();

        for (eleoff = 0; eleoff < ele_size; eleoff ++) {
          long ele = ref_rule_full[eleoff].id;
          if (ele < words) {
            string old_word = dictionary_use[ele];
            if (delete_length < old_word.length()) {
              Ele tempt;
              string new_word = old_word.substr(delete_length);
              if (dictionary_reverse.find(new_word) == dictionary_reverse.end()) {
                long dic_size = dictionary_use.size();
                dictionary_use.insert({dic_size, new_word});
                dictionary_reverse.insert({new_word, dic_size});
              }
              tempt.id = dictionary_reverse[new_word];
              tempt.local_char_offset = local_char_offset;
              hash_numerator += new_word.length();
              new_rule_full.push_back(tempt);
              local_char_offset += new_word.length();
              local_ele_offset ++;
              delete_length = 0;
            }
            else 
              delete_length -= old_word.length();
            
          }
          else {
            long new_rule = -1;
            ulong new_delete_length = rule_delete_from_start(searchFile, ele - words, delete_length, new_rule);
            if (!new_delete_length && new_rule >= 0) {
              Ele tempt;
              tempt.id = new_rule + words;
              tempt.local_char_offset = local_char_offset;
              ulong diff = block_off[splitNum+new_rule][block_off[splitNum+new_rule].size()-1].char_offset;
              local_char_offset += diff;
              hash_numerator += diff * new_rule_full.size();
              new_rule_full.push_back(tempt);
              local_ele_offset ++;
            }
            delete_length = new_delete_length;
          }
          if (delete_length <= 0) {
            eleoff ++;
            local_ele_offset += ele_size - eleoff;
            for (; eleoff < ele_size; eleoff ++) {
              Ele tempt;
              tempt.id = ref_rule_full[eleoff].id;
              tempt.local_char_offset = local_char_offset;
              ulong diff = 0;
              if (eleoff + 1 < ele_size) 
                diff = ref_rule_full[eleoff+1].local_char_offset - ref_rule_full[eleoff].local_char_offset;
              else {
                if (newMid < 0) diff = ref_block_off[newMid+1].char_offset - ref_rule_full[eleoff].local_char_offset;
                else  diff = ref_block_off[newMid+1].char_offset - ref_block_off[newMid].char_offset - ref_rule_full[eleoff].local_char_offset;
              }
              local_char_offset += diff;
              hash_numerator += diff * new_rule_full.size();
              new_rule_full.push_back(tempt);
            }
            vector<vector<Ele>> rf;
            rf.push_back(new_rule_full);
            rule_full.push_back(rf);

            if (newMid >= 0) {
              local_ele_offset += new_block_off[newMid].ele_offset;
              local_char_offset += new_block_off[newMid].char_offset;
            }
            new_block_off[newMid+1].rfid = rules + splitNum;
            new_block_off[newMid+1].hash_numerator = hash_numerator;
            new_block_off[newMid+1].ele_offset = local_ele_offset;
            new_block_off[newMid+1].char_offset = local_char_offset;
            
            for (int nm = newMid+2; nm < new_block_off.size(); nm ++) {
              local_ele_offset += ref_block_off[nm].ele_offset - ref_block_off[nm-1].ele_offset;
              local_char_offset += ref_block_off[nm].char_offset - ref_block_off[nm-1].char_offset;
              new_block_off[nm].ele_offset = local_ele_offset;
              new_block_off[nm].char_offset = local_char_offset;
            
            }
            block_off.push_back(new_block_off);
            new_rule = rules;
            rules ++;
          }
        }
        new_block_off[newMid+1].rfid = rules + splitNum;
        new_block_off[newMid+1].hash_numerator = 0;
        if (newMid < 0) {
          new_block_off[newMid+1].ele_offset = 0;
          new_block_off[newMid+1].char_offset = 0;
        }
        else {
          new_block_off[newMid+1].ele_offset = new_block_off[newMid].ele_offset;
          new_block_off[newMid+1].char_offset = new_block_off[newMid].char_offset;
        }
      }
    }

    if (new_rule < 0 && new_rule_full.size() > 0) { // remain, last block point to this rule
      vector<vector<Ele>> rf;
      rf.push_back(new_rule_full);
      rule_full.push_back(rf);

      int nm = new_block_off.size()-1;
      new_block_off[nm].rfid = rules + splitNum;
      new_block_off[nm].hash_numerator = hash_numerator;
      new_block_off[nm].ele_offset = local_ele_offset;
      new_block_off[nm].char_offset = local_char_offset;
      if (nm > 0){
        new_block_off[nm].ele_offset += new_block_off[nm-1].ele_offset;
        new_block_off[nm].char_offset += new_block_off[nm-1].char_offset;
      }
            
      block_off.push_back(new_block_off);
      new_rule = rules; // mark child
      rules ++;
    }
    
    rule_stack.pop();
    eleoff_stack.pop();
    block_stack.pop();
    
    while(delete_length && rule_stack.size() > 1) {
      long old_rid = rule_stack.top();
      ulong eleoff = eleoff_stack.top();
      int newMid = block_stack.top();
      rule_stack.pop();
      eleoff_stack.pop();
      block_stack.pop();

      ulong local_char_offset = 0;
      ulong local_ele_offset = eleoff;
      ulong hash_numerator = 0;

      vector<BlockOffset> new_block_off;
      vector<BlockOffset> ref_block_off;
      new_block_off.assign(block_off[splitNum+old_rid].begin(), block_off[splitNum+old_rid].end());
      ref_block_off = block_off[splitNum+old_rid];

      vector<Ele> ref_rule_full;
      vector<Ele> new_rule_full;
      // get direct id
      rid = ref_block_off[newMid+1].rfid <= splitNum ? 0 : ref_block_off[newMid+1].rfid - splitNum;
      if (rid >= origin_rules){  
        ref_rule_full = rule_full[splitNum+rid][0];
        new_rule_full.assign(rule_full[splitNum+rid][0].begin(), rule_full[splitNum+rid][0].begin()+eleoff);
      }
      else{  
        ref_rule_full = rule_full[splitNum+rid][newMid+1];
        new_rule_full.assign(rule_full[splitNum+rid][newMid+1].begin(), rule_full[splitNum+rid][newMid+1].begin()+eleoff);
      }
      local_char_offset = ref_rule_full[eleoff].local_char_offset;

      for (long i = 1; i < eleoff; i ++) {
        hash_numerator += (new_rule_full[i].local_char_offset - new_rule_full[i-1].local_char_offset) * (i-1);
      }
      
      if (new_rule >= 0) {  // child
        Ele tmp;
        tmp.id = new_rule + words;
        tmp.local_char_offset = local_char_offset;
        local_char_offset += block_off[splitNum+new_rule][block_off[splitNum+new_rule].size() - 1].char_offset;
        hash_numerator += block_off[splitNum+new_rule][block_off[splitNum+new_rule].size() - 1].char_offset * new_rule_full.size();
        new_rule_full.push_back(tmp);
        local_ele_offset ++;
        eleoff ++;
        new_rule = -1;  // reset
      }
      long ele_size = ref_rule_full.size();
      for (; eleoff < ele_size; eleoff ++) {
        long ele = ref_rule_full[eleoff].id;
        if (ele < words) {
          string old_word = dictionary_use[ele];
          if (delete_length < old_word.length()) {
            string new_word = old_word.substr(delete_length);
            if (dictionary_reverse.find(new_word) == dictionary_reverse.end()) {
              long dic_size = dictionary_use.size();
              dictionary_use.insert({dic_size, new_word});
              dictionary_reverse.insert({new_word, dic_size});
            }
            Ele tempt;
            tempt.id = dictionary_reverse[new_word];
            tempt.local_char_offset = local_char_offset;
            local_char_offset += new_word.length();
            hash_numerator += new_word.length() * new_rule_full.size();
            new_rule_full.push_back(tempt);
            local_ele_offset ++;
            delete_length = 0;
          }
          else
            delete_length -= old_word.length();
        }
        else {
          long new_rule = -1;
          ulong new_delete_length = rule_delete_from_start(searchFile, ele - words, delete_length, new_rule);if (!new_delete_length && new_rule >= 0) {
            Ele tmp;
            tmp.id = new_rule + words;
            tmp.local_char_offset = local_char_offset;
            ulong diff = block_off[splitNum+new_rule][block_off[splitNum+new_rule].size()-1].char_offset;
            local_char_offset += diff;
            hash_numerator += diff * new_rule_full.size();
            new_rule_full.push_back(tmp);
            local_ele_offset ++;
          }
          delete_length = new_delete_length;
        }
        if (delete_length <= 0) {
          eleoff ++;
          local_ele_offset += ele_size - eleoff;
          for (; eleoff < ele_size; eleoff ++) {
            Ele tmp;
            tmp.id = ref_rule_full[eleoff].id;
            tmp.local_char_offset = local_char_offset;
            ulong diff = 0;
            if (eleoff + 1 < ele_size)  diff = ref_rule_full[eleoff+1].local_char_offset - ref_rule_full[eleoff].local_char_offset;
            else {
              if (newMid < 0) diff = ref_block_off[newMid+1].char_offset - ref_rule_full[eleoff].local_char_offset;
              else  diff = ref_block_off[newMid+1].char_offset - ref_block_off[newMid].char_offset - ref_rule_full[eleoff].local_char_offset;
            }
            local_char_offset += diff;
            hash_numerator += diff * new_rule_full.size();
            new_rule_full.push_back(tmp);
          }
          vector<vector<Ele>> rf;
          rf.push_back(new_rule_full);
          rule_full.push_back(rf);

          new_block_off[newMid+1].rfid = rules + splitNum;
          new_block_off[newMid+1].hash_numerator = hash_numerator;
          if (newMid >= 0) {
            local_ele_offset += new_block_off[newMid].ele_offset;
            local_char_offset += new_block_off[newMid].char_offset;
          }
          new_block_off[newMid+1].ele_offset = local_ele_offset;
          new_block_off[newMid+1].char_offset = local_char_offset;
            
          for (int nm = newMid + 2; nm < new_block_off.size(); nm ++) {
            local_ele_offset += ref_block_off[nm].ele_offset - ref_block_off[nm-1].ele_offset;
            local_char_offset += ref_block_off[nm].char_offset - ref_block_off[nm-1].char_offset;
            new_block_off[nm].ele_offset = local_ele_offset;
            new_block_off[nm].char_offset = local_char_offset;
            
          }
          block_off.push_back(new_block_off);
          new_rule = rules;
          rules ++;
        }
      }

      // need to merge blocks
      if (new_rule < 0) {
        int start_newMid = newMid;
        newMid++;
        for (; newMid+1 < ref_block_off.size(); newMid ++) {
          long direct_id = ref_block_off[newMid+1].rfid <= splitNum ? 0 : ref_block_off[newMid+1].rfid - splitNum;
          vector<Ele> ref_rule_full;
          if (direct_id == 0) ref_rule_full = rule_full[searchFile][newMid+1];
          else if (direct_id >= origin_rules) ref_rule_full = rule_full[splitNum+direct_id][0];
          else  ref_rule_full = rule_full[splitNum+direct_id][newMid+1];
          long ele_size = ref_rule_full.size();

          for (eleoff = 0; eleoff < ele_size; eleoff ++) {
            long ele = ref_rule_full[eleoff].id;
            if (ele < words) {
              string old_word = dictionary_use[ele];
              if (delete_length < old_word.length()) {
                Ele tempt;
                string new_word = old_word.substr(delete_length);
                if (dictionary_reverse.find(new_word) == dictionary_reverse.end()) {
                  long dic_size = dictionary_use.size();
                  dictionary_use.insert({dic_size, new_word});
                  dictionary_reverse.insert({new_word, dic_size});
                }
                tempt.id = dictionary_reverse[new_word];
                tempt.local_char_offset = local_char_offset;
                hash_numerator += new_word.length();
                new_rule_full.push_back(tempt);
                local_char_offset += new_word.length();
                local_ele_offset ++;
                delete_length = 0;
              }
              else 
                delete_length -= old_word.length();
              
            }
            else {
              long new_rule = -1;
              ulong new_delete_length = rule_delete_from_start(searchFile, ele - words, delete_length, new_rule);
              if (!new_delete_length && new_rule >= 0) {
                Ele tempt;
                tempt.id = new_rule + words;
                tempt.local_char_offset = local_char_offset;
                ulong diff = block_off[splitNum+new_rule][block_off[splitNum+new_rule].size()-1].char_offset;
                local_char_offset += diff;
                hash_numerator += diff * new_rule_full.size();
                new_rule_full.push_back(tempt);
                local_ele_offset ++;
              }
              delete_length = new_delete_length;
            }
            if (delete_length <= 0) {
              eleoff ++;
              local_ele_offset += ele_size - eleoff;
              for (; eleoff < ele_size; eleoff ++) {
                Ele tempt;
                tempt.id = ref_rule_full[eleoff].id;
                tempt.local_char_offset = local_char_offset;
                ulong diff = 0;
                if (eleoff + 1 < ele_size) 
                  diff = ref_rule_full[eleoff+1].local_char_offset - ref_rule_full[eleoff].local_char_offset;
                else {
                  if (newMid < 0) diff = ref_block_off[newMid+1].char_offset - ref_rule_full[eleoff].local_char_offset;
                  else  diff = ref_block_off[newMid+1].char_offset - ref_block_off[newMid].char_offset - ref_rule_full[eleoff].local_char_offset;
                }
                local_char_offset += diff;
                hash_numerator += diff * new_rule_full.size();
                new_rule_full.push_back(tempt);
              }
              vector<vector<Ele>> rf;
              rf.push_back(new_rule_full);
              rule_full.push_back(rf);

              if (newMid >= 0) {
                local_ele_offset += new_block_off[newMid].ele_offset;
                local_char_offset += new_block_off[newMid].char_offset;
              }
              new_block_off[newMid+1].rfid = rules + splitNum;
              new_block_off[newMid+1].hash_numerator = hash_numerator;
              new_block_off[newMid+1].ele_offset = local_ele_offset;
              new_block_off[newMid+1].char_offset = local_char_offset;
            
              for (int nm = newMid+2; nm < new_block_off.size(); nm ++) {
                start_newMid ++;
                local_ele_offset += ref_block_off[nm].ele_offset - ref_block_off[nm-1].ele_offset;
                local_char_offset += ref_block_off[nm].char_offset - ref_block_off[nm-1].char_offset;
                new_block_off[nm].ele_offset = local_ele_offset;
                new_block_off[nm].char_offset = local_char_offset;
            
              }
              block_off.push_back(new_block_off);
              new_rule = rules;
              rules ++;
            }
          }
          new_block_off[newMid+1].rfid = rules + splitNum;
          new_block_off[newMid+1].hash_numerator = 0;
          if (newMid < 0) {
            new_block_off[newMid+1].ele_offset = 0;
            new_block_off[newMid+1].char_offset = 0;
          }
          else {
            new_block_off[newMid+1].ele_offset = new_block_off[newMid].ele_offset;
            new_block_off[newMid+1].char_offset = new_block_off[newMid].char_offset;
          }
        }
      }

      if (new_rule < 0 && new_rule_full.size() > 0) { // remain, last block point to this rule
        vector<vector<Ele>> rf;
        rf.push_back(new_rule_full);
        rule_full.push_back(rf);

        int nm = new_block_off.size()-1;
        new_block_off[nm].rfid = rules + splitNum;
        new_block_off[nm].hash_numerator = hash_numerator;
        new_block_off[nm].ele_offset = local_ele_offset;
        new_block_off[nm].char_offset = local_char_offset;
        if (nm > 0){
          new_block_off[nm].ele_offset += new_block_off[nm-1].ele_offset;
          new_block_off[nm].char_offset += new_block_off[nm-1].char_offset;
        }
            
        block_off.push_back(new_block_off);
        new_rule = rules;
        rules ++; 
      }
    }
    while (rule_stack.size() > 1) { // delete_length = 0
      long old_rid = rule_stack.top();
      ulong eleoff = eleoff_stack.top();
      int newMid = block_stack.top();
      rule_stack.pop();
      eleoff_stack.pop();
      block_stack.pop();
      
      vector<BlockOffset> new_block_off;
      vector<BlockOffset> ref_block_off;
      new_block_off.assign(block_off[splitNum+old_rid].begin(), block_off[splitNum+old_rid].end());
      ref_block_off = block_off[splitNum+old_rid];

      vector<Ele> ref_rule_full;
      vector<Ele> new_rule_full;
      // get direct id
      rid = ref_block_off[newMid+1].rfid <= splitNum ? 0 : ref_block_off[newMid+1].rfid - splitNum;
      if (rid >= origin_rules){  
        ref_rule_full = rule_full[splitNum+rid][0];
        new_rule_full.assign(rule_full[splitNum+rid][0].begin(), rule_full[splitNum+rid][0].end());
      }
      else{  
        ref_rule_full = rule_full[splitNum+rid][newMid+1];
        new_rule_full.assign(rule_full[splitNum+rid][newMid+1].begin(), rule_full[splitNum+rid][newMid+1].end());
      }

      ulong local_char_move = 0;
      if (eleoff + 1 < ref_rule_full.size())
        local_char_move = ref_rule_full[eleoff+1].local_char_offset - ref_rule_full[eleoff].local_char_offset - block_off[splitNum+new_rule][block_off[splitNum+new_rule].size()-1].char_offset;
      else {
        if (newMid < 0) local_char_move = ref_block_off[newMid+1].char_offset - ref_rule_full[eleoff].local_char_offset - block_off[splitNum+new_rule][block_off[splitNum+new_rule].size()-1].char_offset;
        else  local_char_move = ref_block_off[newMid+1].char_offset - ref_block_off[newMid].char_offset - ref_rule_full[eleoff].local_char_offset - block_off[splitNum+new_rule][block_off[splitNum+new_rule].size()-1].char_offset;
      }
      new_rule_full[eleoff].id = new_rule + words;
      for (vector<Ele>::iterator it=new_rule_full.begin()+eleoff+1; it!=new_rule_full.end(); it++) {
        it->local_char_offset -= local_char_move;
      }
      vector<vector<Ele>> rf;
      rf.push_back(new_rule_full);
      rule_full.push_back(rf);

      new_block_off[newMid+1].rfid = rules + words;
      new_block_off[newMid+1].hash_numerator -= local_char_move * eleoff;
      int i = newMid+1;
      for (vector<BlockOffset>::iterator it=new_block_off.begin()+newMid+1; it!=new_block_off.end(); it++) {
            
        it->char_offset -= local_char_move;
        i++;
      }
      block_off.push_back(new_block_off);
      new_rule = rules;
      rules ++;
    }

    eleoff = eleoff_stack.top();
    newMid = block_stack.top();
    ref_block_off = block_off[searchFile];
    new_block_off.assign(ref_block_off.begin(), ref_block_off.end());
    ref_rule_full = rule_full[searchFile][newMid+1];
    new_rule_full.assign(rule_full[searchFile][newMid+1].begin(), rule_full[searchFile][newMid+1].begin()+eleoff);
    local_char_offset = ref_rule_full[eleoff].local_char_offset;
    local_ele_offset = eleoff;
    hash_numerator = 0;
    for (int i = 1; i < eleoff; i ++) {
      hash_numerator += (ref_rule_full[i].local_char_offset - ref_rule_full[i-1].local_char_offset) * (i-1);
    }

    if (new_rule >= 0) {
      Ele tmp;
      tmp.id = new_rule + words;
      tmp.local_char_offset = local_char_offset;
      local_char_offset += block_off[splitNum+new_rule][block_off[splitNum+new_rule].size()-1].char_offset;
      hash_numerator += block_off[splitNum+new_rule][block_off[splitNum+new_rule].size()-1].char_offset * eleoff;
      new_rule_full.push_back(tmp);
      local_ele_offset ++;
      eleoff ++;
      new_rule = -1;
    }
    if (delete_length <= 0) return 0;

    long ele_size = ref_rule_full.size();
    for (; eleoff < ele_size; eleoff ++) {
      long ele = ref_rule_full[eleoff].id;
      if (ele < words) {
        string old_word = dictionary_use[ele];
        if (delete_length < old_word.length()) {
          string new_word = old_word.substr(delete_length);
          if (dictionary_reverse.find(new_word) == dictionary_reverse.end()) {
            long dic_size = dictionary_use.size();
            dictionary_use.insert({dic_size, new_word});
            dictionary_reverse.insert({new_word, dic_size});
          }
          Ele tempt;
          tempt.id = dictionary_reverse[new_word];
          tempt.local_char_offset = local_char_offset;
          local_char_offset += new_word.length();
          hash_numerator += new_word.length() * new_rule_full.size();
          new_rule_full.push_back(tempt);
          local_ele_offset ++;
          delete_length = 0;
        }
        else
          delete_length -= old_word.length();
      }
      else {
        long new_rule = -1;
        ulong new_delete_length = rule_delete_from_start(searchFile, ele - words, delete_length, new_rule);
        if (!new_delete_length && new_rule >= 0) {
          Ele tmp;
          tmp.id = new_rule + words;
          tmp.local_char_offset = local_char_offset;
          ulong diff = block_off[splitNum+new_rule][block_off[splitNum+new_rule].size()-1].char_offset;
          local_char_offset += diff;
          hash_numerator += diff * new_rule_full.size();
          new_rule_full.push_back(tmp);
          local_ele_offset ++;
        }
        delete_length = new_delete_length;
      }

      if (delete_length <= 0) {
        eleoff ++;
        local_ele_offset += ele_size - eleoff;
        for (; eleoff < ele_size; eleoff ++) {
          Ele tmp;
          tmp.id = ref_rule_full[eleoff].id;
          tmp.local_char_offset = local_char_offset;
          ulong diff = 0;
          if (eleoff + 1 < ele_size)  diff = ref_rule_full[eleoff+1].local_char_offset - ref_rule_full[eleoff].local_char_offset;
          else {
            if (newMid < 0) diff = ref_block_off[newMid+1].char_offset - ref_rule_full[eleoff].local_char_offset;
            else  diff = ref_block_off[newMid+1].char_offset - ref_block_off[newMid].char_offset - ref_rule_full[eleoff].local_char_offset;
          }
          local_char_offset += diff;
          hash_numerator += diff * new_rule_full.size();
          new_rule_full.push_back(tmp);
        }
        rule_full[searchFile][newMid+1] = new_rule_full;

        if (newMid >= 0) {
          local_ele_offset += new_block_off[newMid].ele_offset;
          local_char_offset += new_block_off[newMid].char_offset;
        }
        new_block_off[newMid+1].hash_numerator = hash_numerator;
        new_block_off[newMid+1].ele_offset = local_ele_offset;
        new_block_off[newMid+1].char_offset = local_char_offset;
        for (int nm = newMid + 2; nm < new_block_off.size(); nm ++) {
          local_ele_offset += ref_block_off[nm].ele_offset - ref_block_off[nm-1].ele_offset;
          local_char_offset += ref_block_off[nm].char_offset - ref_block_off[nm-1].char_offset;
          new_block_off[nm].ele_offset = local_ele_offset;
          new_block_off[nm].char_offset = local_char_offset;
        }
        block_off[searchFile]= new_block_off;
        return 0;
      }
    }

    int start_newMid = newMid;
    newMid++;
    for (; newMid+1 < ref_block_off.size(); newMid ++) {
      long direct_id = ref_block_off[newMid+1].rfid <= splitNum ? 0 : ref_block_off[newMid+1].rfid - splitNum;
      vector<Ele> ref_rule_full;
      if (direct_id == 0) ref_rule_full = rule_full[searchFile][newMid+1];
      else if (direct_id >= origin_rules) ref_rule_full = rule_full[splitNum+direct_id][0];
      else  ref_rule_full = rule_full[splitNum+direct_id][newMid+1];
      long ele_size = ref_rule_full.size();

      for (eleoff = 0; eleoff < ele_size; eleoff ++) {
        long ele = ref_rule_full[eleoff].id;
        if (ele < words) {
          string old_word = dictionary_use[ele];
          if (delete_length < old_word.length()) {
            Ele tempt;
            string new_word = old_word.substr(delete_length);
            if (dictionary_reverse.find(new_word) == dictionary_reverse.end()) {
              long dic_size = dictionary_use.size();
              dictionary_use.insert({dic_size, new_word});
              dictionary_reverse.insert({new_word, dic_size});
            }
            tempt.id = dictionary_reverse[new_word];
            tempt.local_char_offset = local_char_offset;
            hash_numerator += new_word.length();
            new_rule_full.push_back(tempt);
            local_char_offset += new_word.length();
            local_ele_offset ++;
            delete_length = 0;
          }
          else 
            delete_length -= old_word.length();
              
        }
        else {
          long new_rule = -1;
          ulong new_delete_length = rule_delete_from_start(searchFile, ele - words, delete_length, new_rule);
          if (!new_delete_length && new_rule >= 0) {
            Ele tempt;
            tempt.id = new_rule + words;
            tempt.local_char_offset = local_char_offset;
            ulong diff = block_off[splitNum+new_rule][block_off[splitNum+new_rule].size()-1].char_offset;
            local_char_offset += diff;
            hash_numerator += diff * new_rule_full.size();
            new_rule_full.push_back(tempt);
            local_ele_offset ++;
          }
          delete_length = new_delete_length;
        }

        if (delete_length <= 0) {
          eleoff ++;
          local_ele_offset += ele_size - eleoff;
          for (; eleoff < ele_size; eleoff ++) {
            Ele tempt;
            tempt.id = ref_rule_full[eleoff].id;
            tempt.local_char_offset = local_char_offset;
            ulong diff = 0;
            if (eleoff + 1 < ele_size) 
              diff = ref_rule_full[eleoff+1].local_char_offset - ref_rule_full[eleoff].local_char_offset;
            else {
              if (newMid < 0) diff = ref_block_off[newMid+1].char_offset - ref_rule_full[eleoff].local_char_offset;
              else  diff = ref_block_off[newMid+1].char_offset - ref_block_off[newMid].char_offset - ref_rule_full[eleoff].local_char_offset;
            }
            local_char_offset += diff;
            hash_numerator += diff * new_rule_full.size();
            new_rule_full.push_back(tempt);
          }
          rule_full[searchFile].erase(rule_full[searchFile].begin()+start_newMid+1, rule_full[searchFile].begin()+newMid+1);
          rule_full[searchFile][start_newMid+1] = new_rule_full;

          if (start_newMid >= 0) {
            local_ele_offset += new_block_off[start_newMid].ele_offset;
            local_char_offset += new_block_off[start_newMid].char_offset;
          }
          new_block_off[start_newMid+1].hash_numerator = hash_numerator;
          new_block_off[start_newMid+1].ele_offset = local_ele_offset;
          new_block_off[start_newMid+1].char_offset = local_char_offset;
          for (int nm = newMid+2; nm < new_block_off.size(); nm ++) {
            start_newMid ++;
            local_ele_offset += ref_block_off[nm].ele_offset - ref_block_off[nm-1].ele_offset;
            local_char_offset += ref_block_off[nm].char_offset - ref_block_off[nm-1].char_offset;
            new_block_off[start_newMid+1].ele_offset = local_ele_offset;
            new_block_off[start_newMid+1].char_offset = local_char_offset;
          }
          new_block_off.erase(new_block_off.begin()+start_newMid+2, new_block_off.end());
          block_off[searchFile] = new_block_off;
          return 0;
        }
      }
      if (newMid >= 0) {
        new_block_off[newMid+1].ele_offset = new_block_off[newMid].ele_offset;
        new_block_off[newMid+1].char_offset = new_block_off[newMid].char_offset;
      }
      else {
        new_block_off[newMid+1].ele_offset = 0;
        new_block_off[newMid+1].char_offset = 0;
      }
      new_block_off[newMid+1].hash_numerator = 0;
    }

  }

  return delete_length;
}


int Query_delete(int searchFile, ulong searchStart_charoff, ulong delete_length) {
  int Rule0Start_eleoff = (searchFile == 0) ? 0 : splitLocation[searchFile - 1];
  int Rule0End_eleoff = splitLocation[searchFile];
  vector<BlockOffset> root_file_blocks = block_off[searchFile];

  if(searchStart_charoff >= root_file_blocks[root_file_blocks.size()-1].char_offset) {
    cout << "WRONG! Range exceeded!\n";
    cout << searchFile << " " << searchStart_charoff << " " << root_file_blocks[root_file_blocks.size()-1].char_offset << endl;
    return -1;
  }
  stack<long> rule_stack;
  stack<ulong> eleoff_stack;
  stack<int> block_stack;
  vector<string> rsts;
  rule_stack.push(0); // add the root rule

  ulong remain = forward_map(searchFile, searchStart_charoff, delete_length, rule_stack, 
                              eleoff_stack, block_stack, root_file_blocks, rsts);
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
  ulong deletLen=64;
  int j=0;
  double total_time = 0;
  clock_t time8, time9;
  for(; j<query_size; j++){
    file_index = rand() % splitNum;
    query_file_indexes[j] = file_index;
    fileLength = block_off[file_index][block_off[file_index].size() - 1].char_offset;
    if (fileLength <= deletLen) {
      continue;
    }
    query_insert_offsets[j] = rand() % (fileLength - deletLen);
    time8 = clock();
    Query_delete(query_file_indexes[j], query_insert_offsets[j], deletLen);
    time9 = clock();
    total_time += (double)(time9 - time8);
  }

  cout << "IO(s) : " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl;
  cout << "DELETE TIME : " << (double)total_time / CLOCKS_PER_SEC << "s" << endl;
	cout << "AVGLatency(s): " << (double)total_time / CLOCKS_PER_SEC / query_size << endl;
	cout << "AVGLatency(us): " << (double)total_time / CLOCKS_PER_SEC / query_size * 1000000 << endl;
	cout << "Throughput(op/s): " << (double)query_size * CLOCKS_PER_SEC / total_time << endl;

  free(query_file_indexes);
  free(query_insert_offsets);
  return 0;
}