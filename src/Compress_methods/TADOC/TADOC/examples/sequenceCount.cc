
#include"common.h"
#include"sequenceWithCount.h"
#include<ctime>

unordered_map<ulong,string> dictionary_use;
int word1,word2,word3;
int word1fID,word2fID,word3fID;
int word1fLoc,word2fLoc,word3fLoc;
set<int> splitSet;
map<string,int> *seqCount;



struct RULE* rule_full;
//bool* ruleok;
int* row;
//int* col;
  vector<int> col;
int words, rules;
int total;

vector<int>* tree;
int* treeSign;

  int splitNum;
int *split ;
int *splitLocation ;


void traversal_basic(  map<string,int> * & seqCount, struct RULE*& rule_full, int & words, int & rules){

  seqCount=new map<string,int>[splitNum];
  int start=0,end=0,splitCur=0;
  for(int j=row[0]; j<row[1]; j++){ // root rule
    int tempt=col[j];
    if(tempt==split[splitCur]&&splitCur<splitNum){
      splitLocation[splitCur]=j;
      splitCur++;
    }
  }// get split index

  start=0,end=0;
  for(int i=0; i<splitNum; i++){
    if(i!=0)
      start=end;
    end=splitLocation[i];

    for(int j=start; j<end; j++){
      int word = col[j];
      if(word>=words){ // for file element (rule)
        dfsDedup(word,i,words); // trim
      }
    }
  }

  start=0,end=0;

  for(int i=0; i<splitNum; i++){
    rule_full[0].fileCount[i]=1; // exist in file 
  }
  for(int i=0; i<splitNum; i++){
    total=0;
    if(i!=0)
      start=end;
    end=splitLocation[i];
    for(int j=start; j<end; j++){ // root element for file index i
      int word = col[j];
      int wordfID = 0;
      if(word < words){ // if is word
        if(splitSet.find(word)==splitSet.end() ){
          total++;

          word1=word2;
          word2=word3;
          word3=word;

          word1fID=word2fID;
          word2fID=word3fID;
          word3fID=wordfID; // file id

        word1fLoc=word2fLoc;
        word2fLoc=word3fLoc;
        word3fLoc=j; // root rule index


          if(total>2){
            char keyStr[100];
            sprintf(keyStr,"%d|%d|%d",word1,word2,word3);
            seqCount[i][string(keyStr)]++;
          }
        } 
      }else{ // if not word
        rule_full[word-words].fileCount[i]++;
        
        dfs(word,i,words); // words for root rule
      }
    }
  }


  for(int i=1; i<rules; i++){ // for rule index
    for(unordered_map<string,int>::iterator j=rule_full[i].seqCount.begin();
        j!=rule_full[i].seqCount.end(); j++){
      for(unordered_map<int,int>::iterator k=rule_full[i].fileCount.begin();
          k!=rule_full[i].fileCount.end(); k++){
        seqCount[k->first][j->first]+=(k->second)*(j->second); // add to seq count
      }
    }
  }

}


int main(int argc, char** argv){

    clock_t t1, t2, t3;
    t1 = clock();
    initialization_basic(argv[1], dictionary_use, rule_full, words, rules);
    t2 = clock();
    traversal_basic(seqCount, rule_full, words, rules);

    double countMulti = 0;
    double countOne = 0;
    double countTotal = 0;
    for (int i = 1; i < rules; i++) {
        for (unordered_map<string, int>::iterator j =
                 rule_full[i].seqCount.begin();
             j != rule_full[i].seqCount.end(); j++) {
            for (unordered_map<int, int>::iterator k =
                     rule_full[i].fileCount.begin();
                 k != rule_full[i].fileCount.end(); k++) {
                if ((j->second) > 1)
                    countMulti += (double)(k->second) * ((double)(j->second));
                if ((j->second) == 1)
                    countOne += (double)(k->second) * ((double)(j->second));
                if ((double)(k->second) * ((double)(j->second)) < 0) {
                    cout << "WRONG!!!!";
                    cout << (k->second) << endl;
                    cout << (j->second) << endl;
                }
            }
        }
  }
  for(int i=0; i<splitNum; i++){
    for(map<string,int>::iterator it=seqCount[i].begin(); it!=seqCount[i].end(); it++){
      countTotal+=(double)(seqCount[i][it->first]);
    }
  }

	t3 = clock();
	cout << "IO(s): " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl;
    cout << "Computation(s): " << (double)(t3 - t2) / CLOCKS_PER_SEC << endl;
    cout << "Total(s): " << (double)(t3 - t1) / CLOCKS_PER_SEC << endl;

  return 0;
}
