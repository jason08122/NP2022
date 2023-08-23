#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <cstring>
#include <bits/stdc++.h>
#include "npshell.h"

using namespace std;

bool is_pipe ( bool head, bool tail )
{
    return head&tail ? false:true;
}

char** str_vec_to_cpp( vector<string> v)
{
    // cout<<"vector siez is "<<v.size()<<endl;
    char** tmp_c = (char**)malloc(sizeof(char*)*(v.size()));
    int tail = 0;
    // cout<<"================="<<endl;
    for ( int i =0;i<v.size();i++ )
    {
        string tmp_str = v[i];
        if ( tmp_str.length() != 0 )
        {
            tail = i;
            char* arr = new char [tmp_str.length() + 1];
            strcpy(arr,tmp_str.c_str());
            // char* c1 = tmp_str;
            tmp_c[i] = arr;
            
            // printf("%s\n",arr);
            // delete [] arr;
        }
        else
        {
            tmp_c[i] = NULL;
        }
        
    }
    // tmp_c[tail+1] = NULL;
    // cout<<"================="<<endl;
    return tmp_c;
}

bool is_np (string _str, int* _pip_num)
{
    // string tmp = *_str;
    // cout<<"is_np"<<endl;
    if ( _str.length() < 2 || (_str[0] != '|' && _str[0] != '!' ) ) 
    {
        // cout<<"aaaaa"<<endl;
        // *_pip_num = 0;
        return false;
    }
    else
    {
        // cout<<"bbbbbbb"<<endl;
        string _num=_str.substr(1,_str.length()-1);
        // *_pip_num = int(_str[1]) - 48;
        *_pip_num = stoi(_num);

        return *_pip_num > 0 ? true:false;
    }
}

vector<string> de_command (parsed_command* _cmd, string* _redirection, string* _separation, int* _pip_num )
{
    vector<string> tmp_p(_cmd->buf_size+1);

    for (int i =0;_cmd->current_index < _cmd-> buf_size ; ++i, ++_cmd->current_index )
    {
        string tmp_str = _cmd->processes[_cmd->current_index];
        // string tmp_string = _cmd->processes[_cmd->current_index];

        if ( tmp_str.length() == 0 ) break;
        // printf("%s\n",tmp_char);
        
        // char* tmp_char = str_to_chrptr(tmp_str);

        if ( tmp_str.compare("|") == 0 || is_np( tmp_str, _pip_num ) )         // ! |
        {
            // cout<<"if"<<endl;
            if ( tmp_str[0] == '!' ) *_separation = "!";
            else *_separation = "|";

            // if ( *_pip_num == 1 && _cmd->current_index !=  _cmd-> buf_size-1 ) 
            // {
            //     *_pip_num = 0 ;
            //     *skip = true;
            // }

            tmp_p[i] = "";
            _cmd->current_index++;
            break;
        }        
        else if ( tmp_str.compare(">") == 0 ) //        >
        {
            _cmd->current_index++;
            
            *_redirection = _cmd->processes[_cmd->current_index];
        }
        else
        {
            tmp_p[i] = tmp_str;
            tmp_p[i+1] = "";
        }        
    }
    return tmp_p;

}

void execute_command( parsed_command cmd )
{
    static np_table _table;
    vector<string> p1;
    vector<string> p2;
    string redirection;
    string separation;
    string np_separation;
    static bool init_table = false;
    int pip_num = 0;
    int read_fd = 0;
    int np_fd = 0;
    bool is_pipe;
    bool head = true;

    if ( !init_table )
    {
        initial_table(&_table);
        init_table = true;
    }
    
    p1 = de_command( &cmd, &redirection, &separation, & pip_num);
    // cout<<"de_command"<<endl;
    update_table(&_table,&np_fd);  
    // cout<<"update"<<endl;
    // cout<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"<<endl;
    // cout<<_table.fd_table[0][0]<<" "<<_table.fd_table[1][0]<<endl;
    // cout<<_table.fd_table[0][1]<<" "<<_table.fd_table[1][1]<<endl;
    // cout<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"<<endl;

    while ( cmd.current_index < cmd.buf_size )
    { 
        // cout<<"while"<<endl;
        if ( pip_num > 0 )
        {
            // cout<<"enp"<<endl;
            execute_np_process(p1, &_table, read_fd,np_fd,separation,pip_num,head);
            pip_num = 0;
            // read_fd = 0;
            // cout<<"de_command"<<endl;
            p2 = de_command( &cmd, &redirection, &separation, & pip_num);  
            update_table(&_table,&np_fd);
            // cout<<"update"<<endl;
            // cout<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"<<endl;
            // cout<<_table.fd_table[0][0]<<" "<<_table.fd_table[1][0]<<endl;
            // cout<<_table.fd_table[0][1]<<" "<<_table.fd_table[1][1]<<endl;
            // cout<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"<<endl;        
            head = true;    
        }
        else if ( separation.compare("|") == 0 )
        {
            // cout<<"|||||||||||||||"<<endl;
            read_fd = excute_pipe_process(p1,read_fd,head,np_separation,np_fd);
            np_fd = 0;
            p2 = de_command( &cmd, &redirection, &separation, & pip_num);
            head = false;
        }            

        

        
        p1.clear();
        p1 = p2;
    }

    if ( pip_num > 0 )
    {
        // cout<<"enp"<<endl;
        execute_np_process(p1, &_table, read_fd,np_fd,separation,pip_num,head);
    }
    else
    {
        // cout<<"ep"<<endl;
        execute_process(p1,redirection,np_separation,np_fd,NULL,read_fd,head,true);
    }    
    if ( read_fd != 0 ) 
    {
        // cout<<"close read_fd"<<endl;
        close(read_fd);
    }
    cmd.processes.clear();
    p1.clear();
    
}

void execute_loop ()  //getline 
{
  while (1)
  {
    cout<<"% ";

    string str;
    getline(cin,str);
    if ( str.length() > 1 )
    {
        parsed_command cmd  (str);

        if ( cmd.processes.size() != 0 ) execute_command(cmd);
    }
    

    // free(&str);
  }
}

int excute_pipe_process(vector<string> tmp_p, int read_fd, bool head, string np_separation, int np_fd)
{
    int* tmp_pipefds = (int*)malloc(sizeof(int)*2);
    // int tmp_pipefds[2];
    int tmp_rfd;
    if ( pipe(tmp_pipefds) == -1 )
    {
        printf("pipe error\n");
    }  

    execute_process(tmp_p,"",np_separation,np_fd, tmp_pipefds, read_fd,head,false);

    tmp_rfd = tmp_pipefds[0];

    if ( read_fd != 0 ) close(read_fd);

    free(tmp_pipefds);

    return tmp_rfd;
}

void execute_process(vector<string> tmp_p, string _redirection, string np_separation, int np_fd,int* pipefd, int infd, bool head, bool tail)
{
    bool is_number_pipe = (np_separation.length() != 0) ? true:false;

    if ( tmp_p[0].compare("exit") == 0 )  // segmentation fault
    {
        EXE_exit();
    }
    else if ( tmp_p[0].compare("printenv") == 0 )
    {
        EXE_printenv(tmp_p);
        return;
    }
    else if ( tmp_p[0].compare("setenv" ) == 0 )
    {
        EXE_setenv(tmp_p);
        return;
    }

    pid_t pid = fork();

    if ( pid == -1 )           //
    {
        Wait(pid);
        pid = fork();
    }
    // cout<<"forked"<<endl;
    switch (pid)
    {
        case -1:
        cout<<"fork error\n";
        break;

        case 0 :
        // child process
        // cout<<"child process"<<endl;
        EXEchild_process(tmp_p,_redirection,np_separation,np_fd, pipefd,infd,head,tail);
        break;

        default:
        // parent process
        EXEparent_process(tmp_p,pid,np_fd, pipefd,infd, is_number_pipe, tail);
        break;
    }

    
}

void execute_np_process (vector<string> tmp_p, np_table* table, int infd, int np_fd, string separation, int pip_line, bool head)
{
    int* pipfd = (int*)malloc(sizeof(int)*2);

    if ( table->fd_table[pip_line][0] == 0)
    {
        pipe(pipfd);

        table->fd_table[pip_line][0] = pipfd[0];
        table->fd_table[pip_line][1] = pipfd[1];
    }
    else
    {
        pipfd[0] = table->fd_table[pip_line][0];
        pipfd[1] = table->fd_table[pip_line][1];
    }

    execute_process(tmp_p,"",separation,np_fd,pipfd,infd,head,false);

    free(pipfd);
}

void execute_head_pipe( int *pipfd, int infd, string np_separation )
{
    // cout<<"head"<<endl;
    close(pipfd[0]);
    dup2(pipfd[1], STDOUT_FILENO );

    if ( np_separation.length() != 0 && np_separation[0] == '!')
    {
        dup2(pipfd[1], STDERR_FILENO);
    }

    close(pipfd[1]);
}

void execute_middle_pipe( int *pipfd, int infd, string np_separation )
{
    // cout<<"middle"<<endl;
    if ( pipfd[0] != 0 ) close(pipfd[0]);
    dup2(infd,STDIN_FILENO);
    dup2(pipfd[1],STDOUT_FILENO);

    if ( np_separation.length() != 0 && np_separation[0] == '!')
    {
        dup2(pipfd[1], STDERR_FILENO);
    }

    close(infd);
    close(pipfd[1]);
}

void execute_tail_pipe( int *pipfd, int infd )
{
    // cout<<"tail"<<endl;
    if ( pipfd != NULL )
    {
        if ( pipfd[0] != 0 ) close(pipfd[0]);
        if ( pipfd[1] != 0 ) close(pipfd[1]);
    }

    dup2(infd, STDIN_FILENO);
    close(infd);
}

void initial_table( np_table* tmp_table)
{
    tmp_table -> table_size = 1005;
    tmp_table -> fd_table = (int**)malloc(sizeof(int*)*1005);

    for ( int i=0;i< tmp_table->table_size;i++ )
    {
        tmp_table -> fd_table[i] = (int*)malloc(sizeof(int)*2);
        tmp_table -> fd_table[i][0] = 0;
        tmp_table -> fd_table[i][1] = 0;
    }
}

void update_table ( np_table* table, int* np_fd )
{
    for ( int i =1;i<table->table_size;i++ )
    {
        table->fd_table[i-1][0] = table->fd_table[i][0];
        table->fd_table[i-1][1] = table->fd_table[i][1];
        table->fd_table[i][0] = 0;
        table->fd_table[i][1] = 0;

    }
    

    if ( table->fd_table[0][0] != 0 )
    {
        *np_fd = table->fd_table[0][0];
        // table->fd_table[0][0] = 0;
        close(table->fd_table[0][1]);
    }
}

void EXEchild_process(vector<string> tmp_p, string _redirection, string np_separation, int np_fd, int* pipefd, int infd, bool head, bool tail)
{
    // cout<<"======================="<<endl;
    // if ( head )
    // {
    //     cout<<"head : true"<<endl;
    // }
    // else
    // {
    //     cout<<"head : false"<<endl;
    // }
    // if ( tail )
    // {
    //     cout<<"tail : true"<<endl;
    // }
    // else
    // {
    //     cout<<"tail : false"<<endl;
    // }
    // cout<<"np_fd : "<<np_fd<<endl;
    // cout<<"======================="<<endl;
    if ( np_fd > 0 ) 
    {
        // cout<<"EXE_np"<<endl;
        EXE_np ( np_fd);
    }

    if ( _redirection.length() != 0 ) 
    {
        EXE_redirect(pipefd,infd,_redirection); 
    }
    if ( !is_pipe(head,tail) && np_fd > 0) 
    {
        // cout<<"close(np_fd)"<<endl;
        close(np_fd);
    }
    if ( !is_pipe(head,tail) || _redirection.length() != 0 ) 
    {
        // cout<<"EXEcvp"<<endl;
        EXEcvp(tmp_p[0],tmp_p);
    }
    if ( is_pipe(head,tail) ) 
    {
        // cout<<"EXEpipe"<<endl;
        EXEpipe(tmp_p,np_separation,pipefd,infd,head,tail);
    }
}

void EXEparent_process(vector<string> tmp_p, pid_t pid, int np_fd, int* pipefd, int infd, bool is_number_pipe, bool tail)
{
    if ( np_fd > 0  ) close(np_fd);
    if ( pipefd != NULL && !is_number_pipe ) close(pipefd[1]);
    if (infd > 0) close(infd);

    if (!tail && is_number_pipe) return;
    else Wait(pid);
}

void EXE_exit()
{
    exit(EXIT_SUCCESS);
}

void EXE_setenv( vector<string> proc )   
{
    if ( proc[1].length() == 0 || proc[2].length() == 0 ) return; 

    setenv(proc[1].c_str(),proc[2].c_str(),1);

}

void EXE_printenv( vector<string> proc)
{
    if ( proc[1].length() != 0 )
    {
        char* c1 = str_to_chrptr(proc[1]);
        char* env = getenv(c1);
        delete [] c1;

        if ( env != NULL )
        {
            printf("%s\n",env);
        }
    }
}

void EXE_redirect ( int *pipefd, int infd, string _redirection )
{
    char* c1 = str_to_chrptr(_redirection);
    int fd = open(c1, O_TRUNC | O_CREAT | O_WRONLY, 0644);

    dup2(fd,STDOUT_FILENO);

    if ( infd > 0 )
    {
        dup2(infd,STDIN_FILENO);
        close(infd);
    }

    if ( pipefd != NULL )
    {
        close(pipefd[0]);
        close(pipefd[1]);
    }

    close(fd);
    delete [] c1;
}

void EXEpipe (vector<string> tmp_p, string np_separation, int* pipefd, int infd, bool head, bool tail )
{
    if ( head )
    {
        execute_head_pipe(pipefd, infd,np_separation);
    }
    else if ( tail )
    {
        execute_tail_pipe(pipefd, infd);
    }
    else
    {
        execute_middle_pipe(pipefd, infd,np_separation);
    }

    EXEcvp(tmp_p[0],tmp_p);
}

void EXE_np ( int np_fd )
{
    dup2(np_fd, STDIN_FILENO);
    close(np_fd);
}

void EXEcvp( string p1 ,vector<string> tmp_p)  //Unknown command: [command].
{
    char* arr = new char [p1.length() + 1];
    strcpy(arr,p1.c_str());

    char** cpp = str_vec_to_cpp(tmp_p);

    if ( execvp(arr,cpp) == -1)
    {
        fprintf(stderr,"Unknown command: [%s].\n",arr);
        exit(EXIT_FAILURE);
    }
}

void Wait(pid_t pid)
{
	int status;
	int waitPID;
	
	while(1)
	{
		waitPID = waitpid(pid, &status, WNOHANG);

		if (waitPID == pid) break;
	}
}

int main(int argc, char* const argv[]) {

    string _path = "PATH";
    string _bin = "bin:.";
    setenv(_path.c_str(), _bin.c_str(), 1);

    execute_loop();
    
    return 0;
}