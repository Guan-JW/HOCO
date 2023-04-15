#include"common.h"
#include"basic.h"
#include<ctime>

unordered_map<ulong,string> dictionary_use; // word id - string 
int* counter;
struct RULE* rule_full;
int words, rules;


void traversal_basic(struct RULE*& rule_full, int & words, int & rules, int* & count){
  rule_full[0].weight=1;
  queue<int> nodeQue;
  nodeQue.push(words); // first root rule
  while(!nodeQue.empty()){
    int head = nodeQue.front();
    nodeQue.pop();
    int rulenum = head - words;
    int tmpSubRuleSize = rule_full[rulenum].ruleIdx.size();
    
    for(int i=0; i<tmpSubRuleSize; i++){ // for each rule
      int theruleis = rule_full[rulenum].ruleIdx[i] - words;
      int numoftherule = rule_full[rulenum].ruleFeq[i];
      rule_full[theruleis].weight+=rule_full[rulenum].weight*numoftherule; // add weight to subrules
      rule_full[theruleis].curInEdge++;
      if(rule_full[theruleis].curInEdge==rule_full[theruleis].numInEdge){
        nodeQue.push(rule_full[rulenum].ruleIdx[i]); // end theruleis, push rule index 
      }
    }
  }
  for(int i=0; i<rules; i++){
    int tmpSubRuleSize = rule_full[i].wordIdx.size();
    for(int j=0; j<tmpSubRuleSize; j++){
        count[rule_full[i].wordIdx[j]] += rule_full[i].wordFeq[j]*rule_full[i].weight; // freq * weight
    }
  }
}


int main(int argc, char** argv){

    clock_t t1, t2, t3;
    t1 = clock();
    initialization_basic(argv[1], dictionary_use, rule_full, words, rules,
                         counter);
    t2 = clock();
    traversal_basic(rule_full, words, rules, counter);
	  t3 = clock();

	  cout << "IO(s): " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl;
    cout << "Computation(s): " << (double)(t3 - t2) / CLOCKS_PER_SEC << endl;
    cout << "Total(s): " << (double)(t3 - t1) / CLOCKS_PER_SEC << endl;


  return 0;
}
