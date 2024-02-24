#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <bits/stdc++.h>
#include "np_simple.h"

using namespace std;

bool is_user_pipe = false;
int clientfd = 0;
char full_command[16000] = "";
np_table client_pipe_table[60];

serv_table client_table
{
    .clientMax = 60,
    .clientSize = 0
};
user_pipe_table pip_table
{
    .pipe_num = 0
};
pool pid_pool
{
    .size = 0
};

bool is_pipe(bool head, bool tail)
{
    return head & tail ? false : true;
}

char **str_vec_to_cpp(vector<string> v)
{
    char **tmp_c = (char **)malloc(sizeof(char *) * (v.size()));
    int tail = 0;
    
    for (int i = 0; i < v.size(); i++)
    {
        string tmp_str = v[i];
        if (tmp_str.length() != 0)
        {
            tail = i;
            char *arr = new char[tmp_str.length() + 1];
            strcpy(arr, tmp_str.c_str());
            
            tmp_c[i] = arr;
        }
        else
        {
            tmp_c[i] = NULL;
        }
    }
    
    return tmp_c;
}

bool is_np(string _str, int *_pip_num)
{
    if (_str.length() < 2 || (_str[0] != '|' && _str[0] != '!'))
    {
        return false;
    }
    else
    {
        string _num = _str.substr(1, _str.length() - 1);
        
        *_pip_num = stoi(_num);

        return *_pip_num > 0 ? true : false;
    }
}

vector<string> individual_process(vector<string> p, string command, int process_nums, int *npfd, int *client_pipe_id)
{
    int index = get_client_index(clientfd);
    int pipe_id = 0;

    int* all_fds = get_all_fds();

    for ( int i=0;i<process_nums;i++ )
    {
        if ( p[i].compare("") == 0 || p[i][0] != '<' ) continue;  

        pipe_id = atoi(&p[i][1]);   // question 

        int flag = get_user_pipe(pipe_id, npfd);

        if ( flag == 1 )
        {
            for ( int i =0;i<client_table.clientSize;i++ )
            {
                if ( all_fds[i] == 0 ) continue;

                dup2(all_fds[i],STDOUT_FILENO);
                printf("*** %s (#%d) just received from %s (#%d) by '%s' ***\n", client_table.clientName[index], index + 1, client_table.clientName[pipe_id], pipe_id + 1, command.c_str());
            }

            free_user_fds(pipe_id);
        }
        else if ( flag == -1)
        {
            printf("*** Error: user #%d does not exist yet.***\n", pipe_id + 1);
			*client_pipe_id = -1;
        }
        else
        {
            printf("*** Error: the pipe #%d->#%d does not exist yet. ***\n", pipe_id + 1, index + 1);
			*client_pipe_id = -1;
        }

        p[i] = "";
    }

    for ( int i =0;i<process_nums;i++ )
    {
        if ( p[i].compare("") == 0 || p[i][0] != '>' ) continue;   

        pipe_id = atoi(&p[i][1]);

        int flag = add_user_pipe(pipe_id);

        if ( flag == 1 )
        {
            is_user_pipe = true;

            for ( int i =0;i<client_table.clientSize;i++ )
            {
                if ( all_fds[i] == 0 ) continue;

                char* tmp_cmmd = new char[command.length()+1];
                strcpy(tmp_cmmd,command.c_str());

                dup2(all_fds[i], STDOUT_FILENO);
                printf("*** %s (#%d) just piped '%s' to %s (#%d) ***\n", client_table.clientName[index], index + 1, tmp_cmmd, client_table.clientName[pipe_id], pipe_id + 1);

                *client_pipe_id = pipe_id;
            }
        }
        else if ( flag == -1 )
        {
            printf("*** Error: user #%d does not exist yet. ***\n", pipe_id +1);
            *client_pipe_id = -1;
        }
        else
        {
            printf("*** Error: the pipe #%d->#%d already exists. ***\n", index + 1, pipe_id + 1);
			*client_pipe_id = -1;
        }

        p[i] = "";
    }

    dup2(clientfd,STDOUT_FILENO);

    return p;
}

vector<string> de_command(parsed_command *_cmd, string *_redirection, string *_separation, int *_pip_num, int *_process_nums)
{
    vector<string> tmp_p(_cmd->buf_size + 1);

    for (int i = 0; _cmd->current_index < _cmd->buf_size; ++i, ++_cmd->current_index)
    {
        string tmp_str = _cmd->processes[_cmd->current_index];

        if (tmp_str.length() == 0)
            break;

        if (tmp_str.compare("|") == 0 || is_np(tmp_str, _pip_num)) // ! |
        {
            if (tmp_str[0] == '!')
                *_separation = "!";
            else
                *_separation = "|";

            tmp_p[i] = "";
            _cmd->current_index++;
            *_process_nums = i;
            break;
        }
        else if (tmp_str.compare(">") == 0) //        >
        {
            _cmd->current_index++;

            *_redirection = _cmd->processes[_cmd->current_index];
        }
        else
        {
            tmp_p[i] = tmp_str;
            tmp_p[i + 1] = "";
            *_process_nums = i + 1;
        }
    }
    return tmp_p;
}

void execute_command(parsed_command cmd, string str)
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
    int process_nums = 0;
    int client_pipe_id = 0;
    bool is_pipe;
    bool head = true;

    strcpy(full_command, str.c_str());

    if (client_pipe_table[clientfd].table_size == 0)
    {
        initial_table(&client_pipe_table[clientfd]);
    }

    p1 = de_command(&cmd, &redirection, &separation, &pip_num, &process_nums);
    // p1 = individual_process(p1, str, process_nums, &np_fd, &client_pipe_id);
    
    if ( client_pipe_id == -1 ) return;

    update_table(&client_pipe_table[clientfd], &np_fd);

    while (cmd.current_index < cmd.buf_size)
    {
        if (pip_num > 0)
        {
            execute_np_process(p1, &client_pipe_table[clientfd], read_fd, np_fd, separation, pip_num, head);
            pip_num = 0;
            
            p2 = de_command(&cmd, &redirection, &separation, &pip_num, &process_nums);
            // p2 = individual_process(p2, str, process_nums, &np_fd, &client_pipe_id);

            if ( client_pipe_id == -1 ) return;

            update_table(&client_pipe_table[clientfd], &np_fd);
            head = true;
        }
        else if (separation.compare("|") == 0)
        {
            read_fd = excute_pipe_process(p1, read_fd, head, np_separation, np_fd);
            np_fd = 0;
            p2 = de_command(&cmd, &redirection, &separation, &pip_num, &process_nums);
            // p2 = individual_process(p2, str, process_nums, &np_fd, &client_pipe_id);

            if ( client_pipe_id == -1 ) return;
            head = false;
        }

        p1.clear();
        p1 = p2;
    }

    // if ( is_user_pipe )
    // {
    //     execute_user_process(p1,read_fd,np_fd,client_pipe_id,head);
    // }
    if (pip_num > 0)
    {
        // is_user_pipe = true;

        execute_np_process(p1, &client_pipe_table[clientfd], read_fd, np_fd, separation, pip_num, head);
    }
    else
    {
        execute_process(p1, redirection, np_separation, np_fd, NULL, read_fd, head, true);
    }
    if (read_fd != 0)
    {
        close(read_fd);
    }
    cmd.processes.clear();
    p1.clear();
}
void get_client_input(int clientfd, char *buffer, int buffer_len)
{
    send(clientfd, "% ", strlen("% "), 0);

    recv(clientfd, buffer, sizeof(char) * buffer_len, 0);
}

void execute_loop(int port) // getline
{
    int bufferLen = 16000;

    while (1)
    {
        int clientfd = 0;

        clientfd = server(port);

        char *buffer = (char *)calloc(bufferLen, sizeof(char));

        while (1)
        {
            get_client_input(clientfd, buffer, bufferLen);

            string str(buffer);

            if (str.length() > 1)
            {
                parsed_command cmd(str);

                if (cmd.processes.size() != 0)
                    execute_command(cmd, str);
            }

            memset(buffer, 0, bufferLen);
        }
    }
}

int excute_pipe_process(vector<string> tmp_p, int read_fd, bool head, string np_separation, int np_fd)
{
    int *tmp_pipefds = (int *)malloc(sizeof(int) * 2);
    int tmp_rfd;

    if (pipe(tmp_pipefds) == -1)
    {
        printf("pipe error\n");
    }

    execute_process(tmp_p, "", np_separation, np_fd, tmp_pipefds, read_fd, head, false);

    tmp_rfd = tmp_pipefds[0];

    if (read_fd != 0)
        close(read_fd);

    free(tmp_pipefds);

    return tmp_rfd;
}

void execute_process(vector<string> tmp_p, string _redirection, string np_separation, int np_fd, int *pipefd, int infd, bool head, bool tail)
{
    bool is_number_pipe = (np_separation.length() != 0) ? true : false;

    if ( ExeBuiltInCommand(tmp_p) == 1 ) return;

    pid_t pid = fork();

    while (pid == -1) //
    {
        Wait(pid);
        pid = fork();
    }
    
    switch (pid)
    {
    case -1:
        cout << "fork error\n";
        break;

    case 0:
        EXEchild_process(tmp_p, _redirection, np_separation, np_fd, pipefd, infd, head, tail);
        break;

    default:
        EXEparent_process(tmp_p, pid, np_fd, pipefd, infd, is_number_pipe, tail);
        break;
    }
}

void execute_user_process ( vector<string> p, int read_fd, int np_fd, int client_fd_id, bool head )
{
    execute_process(p,"","",np_fd,get_user_fds(client_fd_id), read_fd, head, false);
}

void execute_np_process(vector<string> tmp_p, np_table *table, int infd, int np_fd, string separation, int pip_line, bool head)
{
    int *pipfd = (int *)malloc(sizeof(int) * 2);

    if (table->fd_table[pip_line][0] == 0)
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

    execute_process(tmp_p, "", separation, np_fd, pipfd, infd, head, false);

    free(pipfd);
}

void execute_head_pipe(int *pipfd, int infd, string np_separation)
{
    close(pipfd[0]);
    dup2(pipfd[1], STDOUT_FILENO);

    if (np_separation.length() != 0 && np_separation[0] == '!')
    {
        dup2(pipfd[1], STDERR_FILENO);
    }

    close(pipfd[1]);
}

void execute_middle_pipe(int *pipfd, int infd, string np_separation)
{
    if (pipfd[0] != 0)
        close(pipfd[0]);
    dup2(infd, STDIN_FILENO);
    dup2(pipfd[1], STDOUT_FILENO);

    if (np_separation.length() != 0 && np_separation[0] == '!')
    {
        dup2(pipfd[1], STDERR_FILENO);
    }

    close(infd);
    close(pipfd[1]);
}

void execute_tail_pipe(int *pipfd, int infd)
{
    if (pipfd != NULL)
    {
        if (pipfd[0] != 0)
            close(pipfd[0]);
        if (pipfd[1] != 0)
            close(pipfd[1]);
    }

    dup2(infd, STDIN_FILENO);
    close(infd);
}

void initial_table(np_table *tmp_table)
{
    tmp_table->table_size = 1005;
    tmp_table->fd_table = (int **)malloc(sizeof(int *) * 1005);

    for (int i = 0; i < tmp_table->table_size; i++)
    {
        tmp_table->fd_table[i] = (int *)malloc(sizeof(int) * 2);
        tmp_table->fd_table[i][0] = 0;
        tmp_table->fd_table[i][1] = 0;
    }
}

void update_table(np_table *table, int *np_fd)
{
    for (int i = 1; i < table->table_size; i++)
    {
        table->fd_table[i - 1][0] = table->fd_table[i][0];
        table->fd_table[i - 1][1] = table->fd_table[i][1];
        table->fd_table[i][0] = 0;
        table->fd_table[i][1] = 0;
    }

    if (table->fd_table[0][0] != 0)
    {
        *np_fd = table->fd_table[0][0];
        table->fd_table[0][0] = 0;
        close(table->fd_table[0][1]);
    }
}

void EXEchild_process(vector<string> tmp_p, string _redirection, string np_separation, int np_fd, int *pipefd, int infd, bool head, bool tail)
{
    if (np_fd > 0)
    {
        EXE_np(np_fd);
    }

    if (_redirection.length() != 0)
    {
        EXE_redirect(pipefd, infd, _redirection);
    }
    if (!is_pipe(head, tail) && np_fd > 0)
    {
        close(np_fd);
    }
    if (!is_pipe(head, tail) || _redirection.length() != 0)
    {
        EXEcvp(tmp_p[0], tmp_p);
    }
    if (is_pipe(head, tail))
    {
        EXEpipe(tmp_p, np_separation, pipefd, infd, head, tail);
    }
}

void EXEparent_process(vector<string> tmp_p, pid_t pid, int np_fd, int *pipefd, int infd, bool is_number_pipe, bool tail)
{
    AddToPool(pid);

    if (np_fd > 0)
        close(np_fd);
    if (pipefd != NULL && !is_number_pipe)
        close(pipefd[1]);
    if (infd > 0)
        close(infd);

    if (!tail || is_user_pipe || is_number_pipe)
        return;
    else
    {
        Wait(pid);
    }        
}

void EXE_exit()
{
    if ( clientfd > 0 )
    {
        free_pipe_table(&client_pipe_table[clientfd]);
        close(clientfd);
    }

    exit(EXIT_SUCCESS);
}

void EXE_setenv(vector<string> proc)
{
    if (proc[1].length() == 0 || proc[2].length() == 0)
        return;
    
    setenv(proc[1].c_str(), proc[2].c_str(), 1);
}

void EXE_printenv(vector<string> proc)
{
    if (proc[1].length() != 0)
    {
        char *c1 = str_to_chrptr(proc[1]);
        char *env = getenv(c1);
        delete[] c1;

        if (env != NULL)
        {
            printf("%s\n", env);
        }
    }
}

void EXE_redirect(int *pipefd, int infd, string _redirection)
{
    char *c1 = str_to_chrptr(_redirection);
    int fd = open(c1, O_TRUNC | O_CREAT | O_WRONLY, 0644);

    dup2(fd, STDOUT_FILENO);

    if (infd > 0)
    {
        dup2(infd, STDIN_FILENO);
        close(infd);
    }

    if (pipefd != NULL)
    {
        close(pipefd[0]);
        close(pipefd[1]);
    }

    close(fd);
    delete[] c1;
}

void EXEpipe(vector<string> tmp_p, string np_separation, int *pipefd, int infd, bool head, bool tail)
{
    if (head)
    {
        execute_head_pipe(pipefd, infd, np_separation);
    }
    else if (tail)
    {
        execute_tail_pipe(pipefd, infd);
    }
    else
    {
        execute_middle_pipe(pipefd, infd, np_separation);
    }

    EXEcvp(tmp_p[0], tmp_p);
}

void EXE_np(int np_fd)
{
    dup2(np_fd, STDIN_FILENO);
    close(np_fd);
}

void EXEcvp(string p1, vector<string> tmp_p) // Unknown command: [command].
{
    char *arr = new char[p1.length() + 1];
    strcpy(arr, p1.c_str());

    char **cpp = str_vec_to_cpp(tmp_p);

    if (execvp(arr, cpp) == -1)
    {
        fprintf(stderr, "Unknown command: [%s].\n", arr);
        exit(EXIT_FAILURE);
    }
}

void AddToPool ( int pid )
{
    for ( int i=0;i<=pid_pool.size;i++ )
    {
        if ( pid_pool.value[i] == 0 )
        {
            pid_pool.value[i] = pid;

            if ( i == pid_pool.size )
            {
                pid_pool.size++;
            }

            return;
        }
    }
}

void ClearPid ( int pid )
{
    for ( int i=0;i<pid_pool.size;i++ )
    {
        if ( pid_pool.value[i] == 0 ) continue;

        if ( pid_pool.value[i] == pid )
        {
            pid_pool.value[i] = 0;
        } 
    }
}

void Wait(pid_t pid)
{
    int status;
    int waitPID;

    while (1)
    {
        int label = 0;

        for ( int i=0;i<pid_pool.size;i++ )
        {
            waitPID = waitpid(pid_pool.value[i], &status, WNOHANG);

            if ( waitPID == pid_pool.value[i] )
            {
                ClearPid(waitPID);
            }

            if ( waitPID == pid ) return;

            if ( pid == pid_pool.value[i] ) label = 1;
        }

        if ( label == 0 ) return;
    }
}

void free_pipe_table ( np_table* client_pipe_table )
{
    for ( int i = client_pipe_table->table_size-1; i>=0;i-- )
    {
        if ( client_pipe_table->fd_table[i][0] != 0) close (client_pipe_table->fd_table[i][0]);
        if ( client_pipe_table->fd_table[i][1] != 0) close (client_pipe_table->fd_table[i][1]);

        free(client_pipe_table->fd_table[i]);
    }

    free(client_pipe_table->fd_table);
    client_pipe_table->table_size = 0;
}

void free_user_fds( int pipe_id )
{
    for ( int i =0;i<pip_table.pipe_num;i++ )
    {
        if ( pip_table.inIndex[i] == pipe_id && pip_table.outIndex[i] == get_client_index(clientfd) )
        {
            pip_table.inIndex[i] = 0;
            pip_table.outIndex[i] = 0;
            pip_table.inPipe[i] = 0;
            pip_table.outPipe[i] = 0;
            return;
        }
    }
}

int ExeBuiltInCommand (vector<string> p )
{
    if ( p[0].compare("exit") == 0 )
    {
        EXE_exit();
        return 1;
    }
    else if ( p[0].compare("printenv") == 0 )
    {
        EXE_printenv(p);
        return 1;
    }
    else if ( p[0].compare("setenv") == 0 )
    {
        EXE_setenv(p);
        return 1;
    }

    return 0;
}

int set_client_name ( string name )
{
    char* tmp_name = new char[name.length()+1];
    strcpy(tmp_name,name.c_str());


    for ( int i=0;i<client_table.clientSize;i++ )
    {
        if ( strcmp(client_table.clientName[i], tmp_name) == 0 ) return 0;
    }

    strcpy(client_table.clientName[get_client_index(clientfd)], tmp_name);
    delete []tmp_name;    
    return 1;
}

int add_user_pipe ( int pipe_id )
{
    if ( client_table.clientfds[pipe_id] == 0 ) return -1;

    for ( int i=0;i<pip_table.pipe_num;i++ )
    {
        if ( pip_table.inIndex[i] == get_client_index(clientfd) && pip_table.outIndex[i] == pipe_id )
        {
            return 0;
        }
    }

    int pipe_fd[2];

    if ( pipe(pipe_fd) == -1 )
    {
        printf("pipe error\n");
    }

    for ( int i=0;i<pip_table.pipe_num;i++)
    {
        if ( pip_table.inIndex[i] == 0 && pip_table.outIndex[i] == 0)
        {
            pip_table.inIndex[i] = get_client_index(clientfd);
            pip_table.outIndex[i] = pipe_id;
            pip_table.inPipe[i] = pipe_fd[0];
            pip_table.outPipe[i] = pipe_fd[1];

            if ( i == pip_table.pipe_num )
            {
                pip_table.pipe_num++;
                break;
            }
            break;
        }
    }

    return 1;
}

int get_user_pipe ( int pipe_id, int *readfd )
{
    if ( client_table.clientfds[pipe_id] == 0 ) return -1;

    for ( int i =0;i<pip_table.pipe_num;i++)
    {
        if ( pip_table.inIndex[i] == pipe_id && pip_table.outIndex[i] == get_client_index(clientfd) )
        {
            *readfd = pip_table.inPipe[i];
            return 1;
        }
    }

    return 0;
}

int* get_user_fds( int pipe_id )
{
    int* pipe_fd = (int*)malloc(sizeof(int)*2);

    for ( int i=0;i<pip_table.pipe_num;i++ )
    {
        if ( pip_table.inIndex[i] == get_client_index(clientfd) && pip_table.outIndex[i] == pipe_id )
        {
            pipe_fd[0] = pip_table.inPipe[i];
            pipe_fd[1] = pip_table.outPipe[i];

            return pipe_fd;
        }
    }

    return pipe_fd;
}

int* get_all_fds()
{
	return client_table.clientfds;
}

int get_client_index ( int clientfd )
{
    for ( int i =0;i<client_table.clientSize;i++)
    {
        if ( client_table.clientfds[i] == clientfd ) return i;
    }

    return -1;
}

int server(int port)
{
    int sockfd = 0, forClientSockfd = 0,opt = 1;

    printf("Start server at port:%d\n", port);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1)
        printf("create socket failed\n");

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
    {
        printf("setsockopt failed\n");
    }

    struct sockaddr_in serverInfo, clientInfo;
    socklen_t addrlen = sizeof(clientInfo);
    bzero(&serverInfo, sizeof(serverInfo));

    serverInfo.sin_family = PF_INET;
    serverInfo.sin_addr.s_addr = INADDR_ANY;
    serverInfo.sin_port = htons(port);
    bind(sockfd, (struct sockaddr *)&serverInfo, sizeof(serverInfo));
    listen(sockfd, 5);

    while (1)
    {
        forClientSockfd = accept(sockfd, (struct sockaddr *)&clientInfo, &addrlen);

        pid_t pid = fork();

        if (pid != 0)
        {
            close(forClientSockfd);

            int waitPID, status;

            while (1)
            {
                waitPID = waitpid(pid, &status, WNOHANG);

                if (waitPID == pid)
                    break;
            }
        }
        else if (pid == 0)
        {
            clientfd = forClientSockfd;

            dup2(clientfd, STDERR_FILENO);
            dup2(clientfd, STDOUT_FILENO);

            return clientfd;
        }
    }
}

struct sockaddr_in get_client_info ( int client_id )
{
    return client_table.clientInfo[client_id];
}

int main(int argc, char **argv, char **envp)
{
    int port = 8888;

    if (argc > 1)
    {
        port = atoi(argv[1]);
    }
    string _path = "PATH";
    string _bin = "bin:.";
    setenv(_path.c_str(), _bin.c_str(), 1);

    execute_loop(port);

    return 0;
}