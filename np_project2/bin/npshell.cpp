#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>
// #include "parse.h"
#include "execute.h"

using namespace std;

void execute_loop ()
{
  while (1)
  {
    cout<<"% ";

    string s;
    getline(cin,s);

    if ( s == "exit" || s == "EOF" )
    {
      cout<<endl;
      break;
    }

    parsed_command cmd  (s);

    execute_command(cmd);
  }
}

// void execute_command ( parsed_command command )
// {

// }

int main(int argc, char* const argv[]) {

  setenv("PATH", "bin:.", 1);

  execute_loop();
  
  return 0;
}