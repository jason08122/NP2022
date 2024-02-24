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
            string temp = tmp_str;
            string a;
            int index;
            bool _find = false;
            for (int i=0;i<temp.length();i++)
            {
                if ( temp[i] == '+' )
                {
                    _find = true;
                    index = i;
                    break;
                }
            }
            if ( _find )
            {
                int n1 = stoi(temp.substr(index-1,1));
                int n2 = stoi(temp.substr(index+1,1));
                int total = n1 + n2;
                a = "|" + to_string(total);

                this->processes[buf_size] = a;
            }
            else
            {
                this->processes[buf_size] = tmp_str;
            }

            


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

class env_table
{
    public:
    int env_num;
    char env_name[100][1000];
    char env_val[100][1000];
};

class serv_table
{
    public:
    int clientMax;
	int clientSize;
	int clientfds[60];
	char clientName[60][30];
	env_table clientEnv[60];
	struct sockaddr_in clientInfo[60];
};

class user_pipe_table
{
    public:
    int pipe_num;
	int inIndex[3000];
	int outIndex[3000];
	int inPipe[3000];
	int outPipe[3000];
};

class pool
{
    public:
    int size;
    int value[20000];
};

void update_table ( np_table* table, int* np_fd );

void initial_table( np_table* tmp_table);

void get_client_input(int clientfd, char *buffer, int buffer_len);

void EXE_exit();

void EXE_setenv( vector<string> proc );

void EXE_printenv( vector<string> proc);

void EXEchild_process(vector<string> tmp_p, string _redirection, string np_separation, int np_fd, int* pipefd, int infd, bool head, bool tail);

void EXEparent_process(vector<string> tmp_p, pid_t pid, int np_fd, int* pipefd, int infd, bool is_number_pipe, bool tail);

void EXE_name ( vector<string> p);

void EXE_who ( vector<string> p );

void EXE_tell ( vector<string> p );

void EXE_yell ( vector<string> p );

void EXE_redirect ( int *pipefd, int infd, string _redirection );

void EXEcvp( string p1,vector<string> tmp_p );  //Unknown command: [command].

void EXEpipe (vector<string> tmp_p, string np_separation, int* pipefd, int infd, bool head, bool tail );

void EXE_np ( int np_fd );

void execute_process(vector<string> tmp_p, string _redirection, string np_separation, int np_fd,int* pipefd, int infd, bool head, bool tail);

void execute_user_process ( vector<string> p, int read_fd, int np_fd, int client_fd_id, bool head );

void execute_np_process (vector<string> tmp_p, np_table* table, int infd, int np_fd, string separation, int pip_line, bool head);

void execute_head_pipe( int *pipfd, int infd ,string np_separation );

void execute_middle_pipe( int *pipfd, int infd ,string np_separation );

void execute_tail_pipe( int *pipfd, int infd );

void AddToPool ( int pid );

void ClearPid ( int pid );

void Wait(pid_t pid);

void free_pipe_table ( np_table* client_pipe_table );

void free_user_fds( int pipe_id );

void get_client_input ( int clientfd, char* buffer, int buffer_len);

int ExeBuiltInCommand (vector<string> p );

int set_client_name ( string name );

int add_user_pipe ( int pipe_id );

int get_user_pipe ( int pipe_id, int *readfd );

int* get_user_fds( int pipe_id );

int* get_all_fds();

int get_client_index ( int clientfd );

int server(int port);

int excute_pipe_process(vector<string> tmp_p, int read_fd, bool head, string np_separation, int np_fd);

int server (int port );

struct sockaddr_in get_client_info ( int client_id );



