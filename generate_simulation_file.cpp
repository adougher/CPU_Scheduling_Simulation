#include <fstream>
#include <iostream>
#include <ctime>
#include <cstdlib>

using namespace std;

int main( int argc, const char** argv )
{
  ofstream fs;
  fs.open( argv[1], ios::trunc );
  if( fs.is_open( ) == 0 )
  {
    cerr << "Could not open file." << endl;
    exit(EXIT_FAILURE);
  }
  srand (time(NULL));
  fs << argv[2] << " " << 1+rand()%9 << " " << rand()%9 << endl << endl;
  
  int arrival_time=0;
  int procs=atoi(argv[2]);
  for( size_t i=0; i<procs; i++ )
  {
    int threads=1+rand()%9;
    fs << i << " " << rand()%4 << " " << threads << endl;
    for( size_t j=0; j<threads; j++ )
    {
      int CPU_bursts=1+rand()%9;
      fs << arrival_time << " " << CPU_bursts << endl;
      arrival_time+=(1+rand()%5);
      for( int k=0; k<CPU_bursts-1; k++ )
      {
        fs << rand()%10 << " " << rand()%10 << endl;
      }
      fs << rand()%10 << endl << endl;
    }
  }
  fs.close();
}
