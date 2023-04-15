#include"common.h"
double timestamp ()
{
    struct timeval tv;
    gettimeofday (&tv, 0);
    return tv.tv_sec + 1e-6*tv.tv_usec;
}

string getWordDictionary(ifstream& in) {
    int c;

  char t;
    string word;
    t = in.get();
    if( t == ' ' || t == '\t' || t == '\n' ) {
      return string(&t,1);
    }
    word += t;

    while( !in.eof() )
    {
        c = in.get();

        if( c == '\n' ) {
          break;
        }
        word += c;
    }

    return word;
}
