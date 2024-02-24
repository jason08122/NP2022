#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>
#include <bits/stdc++.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>

using namespace std;

char* str_to_chrptr ( string str )
{
    char *cstr = new char[str.length() + 1];
    strcpy(cstr, str.c_str());

    return cstr;
}

class parsed_command 
{
    public:

    int buf_max_size = 1;
    int buf_size = 0;
    int current_index = 0;
    vector<string> processes;
    // char** processes;    
    // bool is_pipe;
    // bool is_np_pipe;
    

    parsed_command ( string input )
    {
        this->processes.resize(this->buf_max_size);

        char* digits = str_to_chrptr(" \f\r\t\v\n");

        char* tmp_char = str_to_chrptr(input);

        char* token = strtok (tmp_char,digits);
        // this->processes = (char**)malloc(sizeof(char*));

        while ( token != NULL )
        {
            // printf("%s\n",token);
            // this->processes[this->buf_size] = (char*)malloc(sizeof(token));
            // this->processes[this->buf_size] = token;
            char* tmp_str = token;
            string str(tmp_str);
            this->processes[buf_size] = tmp_str;


            this->buf_size++;

            if ( this->buf_size >= this->buf_max_size/2 )
            {
                this->buf_max_size *= 2;
                // this->processes = (char**)realloc(this->processes,sizeof(char*) *this->buf_max_size );
                this->processes.resize(this->buf_max_size);
            }

            token = strtok(NULL, digits);
        }
    }   

};

class np_table
{
    public:
    int table_size;
    int** fd_table;
};

void update_table ( np_table* table, int* np_fd );

void initial_table( np_table* tmp_table);

void EXE_exit();

void EXE_setenv( vector<string> proc );

void EXE_printenv( vector<string> proc);

void EXEchild_process(vector<string> tmp_p, string _redirection, string np_separation, int np_fd, int* pipefd, int infd, bool head, bool tail);

void EXEparent_process(vector<string> tmp_p, pid_t pid, int np_fd, int* pipefd, int infd, bool is_number_pipe, bool tail);

void EXE_redirect ( int *pipefd, int infd, string _redirection );

void EXEcvp( string p1,vector<string> tmp_p );  //Unknown command: [command].

void EXEpipe (vector<string> tmp_p, string np_separation, int* pipefd, int infd, bool head, bool tail );

void EXE_np ( int np_fd );

void execute_process(vector<string> tmp_p, string _redirection, string np_separation, int np_fd,int* pipefd, int infd, bool head, bool tail);

void execute_np_process (vector<string> tmp_p, np_table* table, int infd, int np_fd, string separation, int pip_line, bool head);

void execute_head_pipe( int *pipfd, int infd ,string np_separation );

void execute_middle_pipe( int *pipfd, int infd ,string np_separation );

void execute_tail_pipe( int *pipfd, int infd );

void Wait(pid_t pid);

int excute_pipe_process(vector<string> tmp_p, int read_fd, bool head, string np_separation, int np_fd);



