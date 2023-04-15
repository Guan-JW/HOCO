#include <stdio.h>  
#include<iostream>
#include<fstream>
#include <string.h> 
#include <stdlib.h>  
#include <dirent.h>  
#include <sys/stat.h>  
#include <unistd.h>  
#include <vector>
#include <sys/types.h> 
#include <sys/time.h> 

#include<map>
#include "flat_hash_map.hpp"

// #define C_FORM
#define CPP_FORM

using namespace std;

double timestamp ()
{
  struct timeval tv;
  gettimeofday (&tv, 0);
  return tv.tv_sec + 1e-6*tv.tv_usec;
}


string getWord(istream &in);
string getWord(FILE *fp);

map<int, string> dictionary_use;
map<string, int> dictionary_reverse;
// ska::flat_hash_map<string, int> dic;
vector<int> word_sequence;

int word_count = 0;
int word_id = 0;
int file_id = 0;
int no;

int main(int argc, char **argv){ 
	// argv[1] > input file path
	// argv[2] > output dictionary path
	if(argc < 3){
		cout << "please specify the input path and the output path." << endl;
	}

	double t1 = timestamp();
	// string input_file = argv[1];
	// string dic_file = argv[2] + "/dictionary.dic";
	char dic_file[500];
	sprintf(dic_file, "%s/dictionary.dic", argv[2]);

	// memset(word_count, 0, sizeof(int) * 2048);

	// read file
	FILE *fin = fopen(argv[1], "r");
    // ifstream fin(argv[1]);
    if (!fin) {
        cout << "file " << argv[1] << " cannot open." << endl;
        return 0;
    }

    string word;
    while ((word = getWord(fin)).size() > 0) {
        if(dictionary_reverse.find(word) == dictionary_reverse.end()){
			dictionary_use[word_id] = word;
            dictionary_reverse[word] = word_id ++;
        }
        word_count += 1;
        word_sequence.push_back(dictionary_reverse[word]);
    }

	// double time3 = timestamp();
    // cout << "time before write(s) : " << time3 - t1 << endl;

    // write dictionary
#ifdef CPP_FORM
	ofstream fout(dic_file);
	for (auto it : dictionary_use) {
        fout << it.first << " " << it.second << endl;
    }
    fout.close();
#endif

#ifdef C_FORM
	FILE *fout = fopen(dic_file, "w");
    
    for (auto it : dictionary_use) {
        fwrite(&(it.first), sizeof(int), 1, fout);
        fwrite(" ", sizeof(char), 1, fout);
        fwrite((it.second).c_str(), sizeof(char), it.second.size(), fout);
		fwrite("\n", sizeof(char), 1, fout);
    }
    fclose(fout);
#endif

    // numeric original file 
	char out_file[500];
	sprintf(out_file, "%s/output.txt", argv[2]);

#ifdef C_FORM
    FILE *fout_output = fopen(out_file, "w");
	fwrite(&file_id, sizeof(int), 1, fout_output);
	fwrite(" ", sizeof(char), 1, fout_output);

    ulong dic_size = dictionary_use.size();
    fwrite(&dic_size, sizeof(ulong), 1, fout_output);
	fwrite(" ", sizeof(char), 1, fout_output);

    fwrite(&word_count, sizeof(int), 1, fout_output);
	fwrite(" ", sizeof(char), 1, fout_output);

	for(auto it : word_sequence){
        // fout_output << it << " ";
        fwrite(&it, sizeof(int), 1, fout_output);
		fwrite(" ", sizeof(char), 1, fout_output);
    }
    fclose(fout_output);

#endif

#ifdef CPP_FORM
    ofstream fout_output(out_file);
    fout_output << file_id << " " << dictionary_use.size() << " " << word_count << endl;

	for(auto it : word_sequence){
        fout_output << it << " ";
    }
    fout_output.close();
#endif

    double t2 = timestamp();
    cout << "Numeric time(s) : " << t2 - t1 << endl;
    return 0;
}


string getWord(FILE* fp) {
  static char word[99999];
  char* ptr = word;
  char c;                         // current char
  static bool indicator = false;  // flags
  static char old;                // last char
  string myStr;
  if (indicator == true) {  // last time meet a normal char
    indicator = false;
    return string(1, old);  // return the old char as a string
  }
  // every time get a single char
  while ((c = getc(fp)) != EOF) {
    if (!(c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == ',' ||
          c == '.')) {
      // if not a boundary just return the char
      *ptr++ = c;        // add c to the buffer
      indicator = true;  // set the indicator
    } else {
      // arrive the boundary c between words, last time not meet a normal char
      if (indicator == false) return string(1, c);
      // if have met a boundary, last time meet a normal char
      old = c;
      *ptr++ = '\0';  // end the string (not include the boundary chars)
      ptr = word;     // why not just return string(word) ?
      return string(ptr);
    }
  }
  return myStr;  // when meet the end, return a null string
}

string getWord(istream& in)
{
  char c;
  static bool has_special=false;
  static char special;
  if(has_special==true){
    has_special=false;
    return string(&special);
  }

  string word;
    c = in.get();
  while( !in.eof() )
  {


    if( c == ' ' || c == '\t' || c == '\n' ) {
      special=c;
      has_special=true;
      break;
    }
    word += c;
    c = in.get();
  }
  if((word.size()==0)&&(has_special==true)){
    has_special=false;
    return string(&special);
  }
  return word;
}