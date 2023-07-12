/****************************************************************************

 sequitur.cc - Module containing the main() function, and functions for
               printing out the grammar.

    main() function contains: command line options parsing,
    program initialization, reading the input...

 Compilation notes:
    Dedicfilee PLATFORM_MSWIN macro if compiling this program under Windows,
    or PLATFORM_UNIX if compiling it under Unix/Linux.

 Program usage (syntax):
    See "help" string below (or run "sequitur -h" after compiling the program).

 Additional notes:
    Progress indicator under MS Windows is yet to be done.

 ***************************************************************************/


#ifdef PLATFORM_MSWIN
#include <fcntl.h>    // for O_BINARY
#endif

#include <unistd.h>
#include <sys/timeb.h>

#ifdef PLATFORM_UNIX
#include <sys/times.h>
#endif

#include <limits.h>
#include "classes.h"
#include "sequitur.h"

#include<fstream>

#include <sys/time.h>
using namespace std;

double timestamp ()
{
  struct timeval tv;
  gettimeofday (&tv, 0);
  return tv.tv_sec + 1e-6*tv.tv_usec;
}

map<ulong,string> dictionary_use;
map<string,ulong> dictionary_reverse;
  int fileid = 0;

void print_wc(int** row, int** col);
string getWordDictionary(ifstream& in)
{
    int c;
  char t;
    string word;
    t = in.get();
    if( t == ' ' || t == '\t' || t == '\n' || t == '\r' || t == ',' || t == '.') {
      return string(1,t);
    }
    word += t;
    while( !in.eof() ){
        t = in.get();
        if( t == '\n' ) {
          break;
        }
        word += t;
    }
    return word;
}



void calculate_rule_usage(rules *r)
{
  for (symbols *p = r->first(); !p->is_guard(); p = p->next()) {
    if (p->non_terminal()) {
      p->rule()->usage(1);
      calculate_rule_usage(p->rule());
    }
  }
}


rules *S;                 // pointer to main rule of the grammar

int num_rules = 0;        // number of rules in the grammar
int num_symbols = 0;      // number of symbols in the grammar
int min_terminal,         // minimum and maximum value among terminal symbols
    max_terminal;         //
int max_rule_len = 2;     // maximum rule length
bool compression_initialized = false;

int compress = 0,
  do_uncompress = 0,
  do_print = 0,
  reproduce = 0,
  quiet = 0,
  phind = 0,
  numbers = 0,
  print_rule_freq = 0,
  print_rule_usage = 0,
  delimiter = -1,
  memory_to_use = 1000000000,

  // minimum number of times a digram must occur to form rule minus one
  // (e.g. if K is 1, two occurrences are required to form rule)
    K = 1;

char *delimiter_string = 0;
// new
string output_path;

void uncompress(), print(), number(), forget(symbols *s),
  forget_print(symbols *s);
void start_compress(bool), end_compress(), stop_forgetting();
ofstream *rule_S = 0;

#ifdef PLATFORM_MSWIN
extern "C" {
int getopt ( int argc, char **argv, char *optstring );
}
#endif

const char *help = "\n\
usage: sequitur -cdpqrtTuz -k <K> -e <delimiter> -f <max symbols> -m <memory_limit>\n\n\
-p    print grammar at end\n\
-d    treat input as symbol numbers, one per line\n\
-c    compress\n\
-u    uncompress\n\
-m    use this amount of memory, in MB, for the hash table (default 1000)\n\
-q    quiet: suppress progress numbers on stderr\n\
-r    reproduce full expansion of rules after each rule\n\
-t    print rule usage in the grammar after each rule\n\
-T    print rule usage in the input after each rule\n\
-z    put rule S in file called S, other rules on stdout as usual\n\
-k    set K, the minmum number of times a digram must occur to form rule\n\
      (default 2)\n\
-e    set the delimiter symbol. Rules will not be formed across (i.e. \n\
      including) delimiters. If with -d, 0-9 are treated as numbers\n\
-f    set maximum symbols in grammar (memory limit). Grammar/compressed output\n\
      will be generated once the grammar reaches this size\n\
";


// for print the rules out
rules **R1;
int Ri;

extern "C" int sequitur(string out_path, map<string, int>& dic, 
                          vector<int> & input, int** row, int** col, 
                          int* rule_num, int doCompress)
{
  cout << "sequitur" << endl;
  
  output_path = out_path;
  if (doCompress == 2)  do_uncompress = 1;
  extern char *optarg;         // these are...
  extern int optind, opterr;   // ... from getopt()

  // number of input characters read so far (used only for progress indicator)
  int chars = 0;
  // maximum number of symbols that can be held in memory (used with -f option)
  int max_symbols = 0;

  int c;

  //if(0){//for saving time zf 20180121
  if(doCompress == 1){
    cout << "do compress" << endl;
    numbers=1;
    for (map<string, int>::iterator i = dic.begin(); 
                    i != dic.end(); i ++) {
      dictionary_use[i->second] = i->first;
      dictionary_reverse[i->first] = i->second;
    }
  }

  if (K < 1) {
    cerr << "sequitur: k must be at least 2" << endl;
    exit(1);
  }

  if (delimiter_string)
    delimiter = numbers ? atoi(delimiter_string) : delimiter_string[0];


  //
  // if on MS Windows, set stdin and stdout to binary mode
  //    (on Unix, this is the default, hence setting is not necessary)
  //

#ifdef PLATFORM_MSWIN
  if (setmode(fileno(stdin), O_BINARY) == -1)
  {
      cerr << "sequitur: can't set binary mode on standard input" << endl;
      return 2;
  }
  if (setmode(fileno(stdout), O_BINARY) == -1)
  {
      cerr << "sequitur: can't set binary mode on standard output" << endl;
      return 2;
  }
#endif


  //
  // do some initializations
  //

  if (do_uncompress) {
    uncompress();
    exit(0);
  }

  if (phind) current_rule = 1;

  S = new rules;


  //
  // read first character and put it in the grammar
  //

  int i;
  int wid = 0;

  // cin >> fileid;
  // int sizeDic;//useless
  // cin >> sizeDic;//useless
  // int wordTotal;//useless
  // cin >> wordTotal;//useless

  int sizeDic = dictionary_use.size();
  int wordTotal = input.size();

  i = input[wid ++];
  min_terminal = max_terminal = i;

  S->last()->insert_after(new symbols(i));



  //
  // now loop reading characters (loop will end upon reaching end of input)
  //
  // static int last_time = 0;
  // struct timeb tp;
  // ftime(&tp);
  // last_time =  tp.time * 1000 + tp.millitm;

  while (1) {

// // progress indicator
// #ifdef PLATFORM_UNIX
//     if (++ chars % 1000000 == 0 && !quiet) {
//       struct tms buffer;
//       extern int collisions, lookups, occupancy;//, occ[100];

//       ftime(&tp);
//       int milliseconds =  tp.time * 1000 + tp.millitm;

//     //   fprintf(stderr, "%3d MB processed, %.2f MB/s, %.3f collisions/lookup, %.2f%% occupancy\n",
// 	//       chars / 1000000, 1000.0 / (milliseconds - last_time),
// 	//       collisions/ float(lookups), 100.0 * occupied / table_size);
//       //      last_time = buffer.tms_utime;
//       last_time = milliseconds;
//     }
// #endif

    // read a character, if on end of input exit loop
    if (wid == input.size())  break;
    i = input[wid ++];

    if (i < min_terminal) min_terminal = i;
    else if (i > max_terminal) max_terminal = i;

    // append read character to end of rule S, and enforce constraints
    S->last()->insert_after(new symbols(i));
    S->last()->prev()->check();

    // if memory limit reached, "forget" part of the grammar
    if (max_symbols && num_symbols > max_symbols)
      if (compress) {
         // if compression has not been initalized, initialize
         if (!compression_initialized) {
            start_compress(false); compression_initialized = true;
         }
         // send first symbol of (the remaining part of) the grammar
	 // to the compressor
         forget(S->first());
      }
      else if (phind) forget_print(S->first());
  }

  // now all input has been read,
  // proceed to compression/printing out the grammar


  // initialize compression
  if (compress && !compression_initialized) start_compress(true);

  void calculate_rule_usage(rules *r);
  if (print_rule_usage) calculate_rule_usage(S);

  if (max_symbols || compress || phind) {
    // tell the compressor no more rules will be removed from memory
    if (compress) stop_forgetting();
    while (S->first()->next() != S->first())
      // send symbol to the compressor
      if (compress) forget(S->first());
      else if (phind) forget_print(S->first());
  }

  if (compress) end_compress();

  if (do_print) {
     number();
     print();
  }


  if(doCompress == 1){
     number();
     rule_num[0] = Ri;
     print_wc(row, col);
  }
    return 0;
}


// for print the rules out

void print_wc(int** row, int** col)
{

  (*row)=(int*)malloc(sizeof(int)*(1+Ri));
  memset((*row),0,sizeof(int)*(1+Ri));
  for (int i = 0; i < Ri; i ++) {
    for (symbols *p = R1[i]->first(); !p->is_guard(); p = p->next())
      (*row)[i+1]++;
  }
  // for (int i = 0; i < 5; i ++)
  //   cout << "row-i: " << (*row)[i] << endl;
  for (int i = 0; i < Ri; i ++) {
    (*row)[i+1]+=(*row)[i];
  }

  int counttmp=0;
  (*col)=(int*)malloc(sizeof(int)*(*row)[Ri]);
  for (int i = 0; i < Ri; i ++) {
    for (symbols *p = R1[i]->first(); !p->is_guard(); p = p->next())
      if (p->non_terminal()) {
        (*col)[counttmp++]=(p->rule()->index() + dictionary_use.size());
      }
      else {
        (*col)[counttmp++]=((p->s)-1)/2 ;
      }
  }
  // cout<<"Words: "<< dictionary_use.size()<<endl;
}



// print out the grammar (after non-terminals have been numbered)
void print()
{
  for (int i = 0; i < Ri; i ++) {
    cout << i << " -> ";
    for (symbols *p = R1[i]->first(); !p->is_guard(); p = p->next())
      if (p->non_terminal()) cout << p->rule()->index() << ' ';
      else cout << *p << ' ';
    if (i > 0 && print_rule_freq) cout << '\t' << R1[i]->freq();
    if (i > 0 && print_rule_usage) cout << "\t(" << R1[i]->usage() << ")";
    if (reproduce && i > 0) {
      cout << '\t';
      R1[i]->reproduce();
    }
    cout << endl;
  }

  if (print_rule_freq)
    cout << (num_symbols - Ri) << " symbols, " << Ri << " rules "
	 << (num_symbols * (sizeof(symbols) + 4) + Ri * sizeof(rules))
	 << " total space\n";

}

// number non-terminal symbols for printing
void number()
{
  R1 = (rules **) malloc(sizeof(rules *) * num_rules);
  memset(R1, 0, sizeof(rules *) * num_rules);
  R1[0] = S;
  Ri = 1;

  for (int i = 0; i < Ri; i ++)
    for (symbols *p = R1[i]->first(); !p->is_guard(); p = p->next())
      if (p->non_terminal() && R1[p->rule()->index()] != p->rule()) {
         p->rule()->index(Ri);
         R1[Ri ++] = p->rule();
      }
}

// **************************************************************************
// print out symbol of rule S - and, if it is a non-terminal, its rule's right hand -
// printing rule S to separate file
// **************************************************************************
void forget_print(symbols *s)
{
  // open file to write rule S to, if not already open
  if (rule_S == 0) rule_S = new ofstream("S");

  // symbol is non-terminal
  if (s->non_terminal()) {

    // remember the rule the symbol heads and delete the symbol
    rules *r = s->rule();
    delete s;

    // there are no more instances of this symbol in the grammar (rule contents has already been printed out),
    // so we print out the symbol and delete the rule
    if (r->freq() == 0) {
      *rule_S << r->index();
      // delete all symbols in the rule
      while (r->first()->next() != r->first())
	delete r->first();
      // delete rule itself
      delete r;
    }
    // there are still instances of this symbol in the grammar
    else {
      // print rule's right hand (to stdout) (this will change r->index())
      if (r->index() == 0)
	r->output();
      // print rule index, i.e. non-terminal symbol, to file for rule S
      *rule_S << r->index();
    }

  }
  // symbol is terminal (print it and delete it)
  else {
    *rule_S << *s;
    delete s;
  }

  // put space between symbols
  *rule_S << ' ';
}
