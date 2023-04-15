#include <stdio.h>  
#include<iostream>
#include<fstream>
#include <string.h> 
#include <stdlib.h>  
#include <dirent.h>  
#include <sys/stat.h>  
#include <unistd.h>  
#include <sys/types.h> 
#include <sys/time.h> 

#include<map>
using namespace std;

double timestamp ()
{
  struct timeval tv;
  gettimeofday (&tv, 0);
  return tv.tv_sec + 1e-6*tv.tv_usec;
}

int numberi;
int fileid;
int status = mkdir("./output", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    // ofstream outRelation("./output/relation.txt");

map<ulong,string> dictionary_use;
map<string,ulong> dictionary_reverse;

int wordCount[2048];
int wordCountFID;

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
void listDir(char *path)  
{  
  DIR              *pDir ;  
  struct dirent    *ent  ;  
  int               i=0  ;  
  char              childpath[512];  


  pDir=opendir(path);  
  memset(childpath,0,sizeof(childpath));  
  sprintf(childpath,"%s",path);

  while((ent=readdir(pDir))!=NULL)  
  {  

    if(ent->d_type & DT_DIR)  
    {  

      if(strcmp(ent->d_name,".")==0 || strcmp(ent->d_name,"..")==0)  
        continue;  

      sprintf(childpath,"%s/%s",path,ent->d_name);  

      listDir(childpath);  
      sprintf(childpath,"%s",path);

    }  
    else{
      char              pathTmp[512];  
      sprintf(pathTmp,"%s/%s",childpath,ent->d_name);  
      cout<<"->"<<pathTmp<<endl;
      ifstream in(pathTmp);
      string tempt ;
      while(!(in.eof())){
        tempt = getWord(in);
        if(dictionary_reverse.find(tempt)!=dictionary_reverse.end()){
        }
        else{
          dictionary_use[numberi]=tempt;
          dictionary_reverse[tempt]=numberi++;
        }
        wordCount[ wordCountFID ] ++;
      }
      wordCountFID++;
    }
  }  

}  


void listDir2(char *path)  
{  
  DIR              *pDir ;  
  struct dirent    *ent  ;  
  int               i=0  ;  
  char              childpath[512];  


  pDir=opendir(path);  
  memset(childpath,0,sizeof(childpath));  
  sprintf(childpath,"%s",path);

  while((ent=readdir(pDir))!=NULL)  
  {  

    if(ent->d_type & DT_DIR)  
    {  

      if(strcmp(ent->d_name,".")==0 || strcmp(ent->d_name,"..")==0)  
        continue;  

      sprintf(childpath,"%s/%s",path,ent->d_name);  

      listDir(childpath);  
      sprintf(childpath,"%s",path);

    }  
    else{
      char              pathTmp[512];  
      sprintf(pathTmp,"%s/%s",childpath,ent->d_name);  
      cout<<"fileID: "<<fileid<<" -> "<<pathTmp<<endl;
// outRelation<<fileid<<" "<<ent->d_name<<endl;

      ifstream in(pathTmp);

      char outPath[512];
      sprintf(outPath,"./output/%d",fileid);  
      ofstream out2(outPath);
      string tempt ;

      out2<<fileid<<" "<<dictionary_use.size()<<" "<<wordCount[fileid]<<endl;

      int countTmp = 0;
      while(!(in.eof())){
        tempt = getWord(in);
        if(dictionary_reverse.find(tempt)!=dictionary_reverse.end()){
          out2<<dictionary_reverse[tempt]<<" ";
          countTmp++;
        }
        else{
          cout<<"error! zf\n";
          exit(-1);
        }
      }
      out2.close();
      if(countTmp != wordCount[fileid]) {cout<<countTmp<<" "<<wordCount[fileid]<<"wrong!   zf\n";exit(-1);}
      fileid++;
    }
  }  

}


int main(int argc,char *argv[])  
{  
  numberi = 0;
  fileid = 0;

  memset(wordCount, 0, sizeof(int)*2048);
wordCountFID=0;

  cout<<"traversing graph"<<endl;
  double t1=timestamp();
  listDir(argv[1]);  

  cout<<"generating dic"<<endl;
  double t2=timestamp();

  ofstream fout("./output/dictionary.dic");
  for(map<ulong,string>::iterator it=dictionary_use.begin();
      it!=dictionary_use.end();
        it++){
      fout<<it->first<<" "<<it->second<<endl;
    }
    fout.close();


  cout<<"generating output"<<endl;
  double t3=timestamp();

    listDir2(argv[1]);

    // outRelation.close();

  cout<<"finished"<<endl;
  double t4=timestamp();

  cout<<"traversing graph:"<<t2-t1<<endl;
  cout<<"dictionary generate:"<<t3-t2<<endl;
  cout<<"output generate:"<<t4-t3<<endl;
  cout<<"total time:"<<t4-t1<<endl;

	return 0;  
}
