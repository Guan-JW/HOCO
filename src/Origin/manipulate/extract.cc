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
#include<algorithm>

#include<fstream>
#include <fcntl.h>    // for O_BINARY
#include <unistd.h>
#include <sys/times.h>
#include <limits.h>
#include <ctime>

using namespace std;

double timestamp ()
{
    struct timeval tv;
    gettimeofday (&tv, 0);
    return tv.tv_sec + 1e-6*tv.tv_usec;
}

int main(int argc, char** argv){
  clock_t t1,t2;
  t1 = clock();
  char relationDir[100];
  sprintf(relationDir,"%s",argv[1]);
  
    string line, all;
    ifstream myfile (relationDir);
    if (myfile.is_open())
    {
      int a=0;
      while ( getline (myfile,line) )
      {
        all.append(line);
        string b="\n";
        all.append(b);
      }
      myfile.close();
    }

    else cout << "Unable to open file"; 

    t2 = clock();

    int fileLength=all.size();
    int totalJ=100;
    int searchLen=164;

    double time6=timestamp();
    for(int j=0; j<totalJ; j++){
      int searchStart=rand()%(fileLength-searchLen);
      string rst = "";
      for (int i=0; i<searchLen; i++)   
        rst += all[searchStart+i];
    }
    double time7=timestamp();

    cout << "IO(s) : " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl;
    cout<<"AVGLatency(s): "<<(time7-time6)/totalJ<<endl;
    cout<<"AVGLatency(us): "<<(time7-time6)/totalJ*1000000<<endl;
    cout<<"totalTime(s): "<<(time7-time6)<<endl;
    cout<<"Throughput(op/s): "<<(totalJ)/(time7-time6)<<endl;

    return 0;
}
