#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <bits/stdc++.h>
#include "np_multi_proc.h"
#include <sys/shm.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/ipc.h>
#include <errno.h>
#include <string.h>

using namespace std;

bool is_user_pipe = false;
bool is_number_pipe = false;
int clientfd = 0;

char full_command[16000] = "";
np_table client_pipe_table;
serv_table* serv_table_ptr ;
user_pipe_table* user_Ptable_ptr;
pool* pool_ptr;
int* speaker_idx;
int* listener_idx;
msg_info* msg;
bool init = false;


// serv_table client_table
// {
//     .clientMax = 60,
//     .clientSize = 0
// };
// user_pipe_table pip_table
// {
//     .pipe_num = 0
// };
// pool pid_pool
// {
//     .size = 0
// };

void remove_enter( char arr[])
{
    for ( int i =0;i<16000;i++ )
    {
        if ( arr[i] == '\n' ) arr[i] = 0;break;
    }
}

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

vector<string> individual_process(vector<string> p, char* command, int process_nums, int *npfd, int *client_pipe_id)
{
    int index = get_client_index(clientfd);
    int pipe_id = 0;
    int* all_fds = get_all_fds();
    // serv_table* serv_table_ptr ;

    

    for ( int i=0;i<process_nums;i++ )
    {
        if ( p[i].compare("") == 0 || p[i][0] != '<' ) continue;  

        pipe_id = atoi(&p[i][1]) -1;   // question 

        int flag = get_read_fifo(pipe_id, npfd);

        if ( flag == 1 )
        {
            msg->flag = 4;
            strcpy(msg->full_msg, command);
            *speaker_idx = pipe_id;
            *listener_idx = index;

            for ( int i =0;i<serv_table_ptr->clientSize;i++ )
            {
                if ( all_fds[i] == 0 ) continue;

                kill(serv_table_ptr->p_table[i], SIGUSR1);
                
            }
            
            user_Ptable_ptr->fifo_exist[pipe_id][index] = false;
        }
        else if ( flag == -1)
        {
            printf("*** Error: user #%d does not exist yet. ***\n", pipe_id + 1);
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
        if ( p[i].compare("") == 0 || p[i][0] != '>' ) continue;   // what about > 

        pipe_id = atoi(&p[i][1]) -1;

        int flag = add_fifo(pipe_id);

        if ( flag == 1 )
        {
            is_user_pipe = true;
            msg->flag = 3;
            strcpy(msg->full_msg, command);
            *speaker_idx = index;
            *listener_idx = pipe_id;

            for ( int i =0;i<=serv_table_ptr->clientSize;i++ )
            {
                // printf("client%d fd is: %d\n", i+1, all_fds[i]);
                if ( all_fds[i] == 0 ) continue;
                
                kill(serv_table_ptr->p_table[i], SIGUSR1);
            }
            
            *client_pipe_id = pipe_id;
            user_Ptable_ptr->fifo_exist[index][pipe_id] = true;
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

    // printf("ffffffff index1 is %d\nindex2 is %d\n", *speaker_idx, *listener_idx);
    // dup2(clientfd,STDOUT_FILENO);

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

void execute_command(parsed_command cmd, char* whole_cmd)
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
    is_user_pipe = false;
    is_number_pipe = false;

    strcpy(full_command, whole_cmd);

    if (client_pipe_table.table_size == 0)
    {
        initial_table(&client_pipe_table);
    }

    p1 = de_command(&cmd, &redirection, &separation, &pip_num, &process_nums);
    p1 = individual_process(p1, whole_cmd, process_nums, &np_fd, &client_pipe_id);
    // printf("iiiiiiiiiiiii index1 is %d\nindex2 is %d\n", *speaker_idx, *listener_idx);
    if ( client_pipe_id == -1 ) return;

    update_table(&client_pipe_table, &np_fd);

    while (cmd.current_index < cmd.buf_size)
    {
        if (pip_num > 0)
        {
            execute_np_process(p1, &client_pipe_table, read_fd, np_fd, separation, pip_num, head);
            pip_num = 0;
            
            p2 = de_command(&cmd, &redirection, &separation, &pip_num, &process_nums);
            p2 = individual_process(p2, whole_cmd, process_nums, &np_fd, &client_pipe_id);

            if ( client_pipe_id == -1 ) return;

            update_table(&client_pipe_table, &np_fd);
            
            head = true;
        }
        else if (separation == "|" || is_user_pipe )
        {
            read_fd = excute_pipe_process(p1, read_fd, head, np_separation, np_fd);
            np_fd = 0;
            p2 = de_command(&cmd, &redirection, &separation, &pip_num, &process_nums);
            p2 = individual_process(p2, whole_cmd, process_nums, &np_fd, &client_pipe_id);

            if ( client_pipe_id == -1 ) return;
            head = false;
        }

        p1.clear();
        p1 = p2;
    }
    // printf("eeeeee index1 is %d\nindex2 is %d\n", *speaker_idx, *listener_idx);
    if ( is_user_pipe )
    {
        execute_user_process(p1,read_fd,np_fd,client_pipe_id,head);
        // int tmp_fd = open(msg->file_path, O_WRONLY);
        // close(tmp_fd);
    }
    else if (pip_num > 0)
    {
        is_number_pipe = true;
        execute_np_process(p1, &client_pipe_table, read_fd, np_fd, separation, pip_num, head);
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
void get_client_input(int _clientfd, char *buffer, int buffer_len)
{
    send(_clientfd, "% ", strlen("% "), 0);

    recv(_clientfd, buffer, sizeof(char) * buffer_len, 0);
}

void execute_loop(int port) // getline
{
    while (1)
    {
        server(port);
    }

    
}

void send_login_message( int _clientfd, struct sockaddr_in clientInfo)
{
    char ipv4[20];

    inet_ntop(AF_INET, &clientInfo.sin_addr, ipv4, sizeof(struct sockaddr));

    int prev_fd = dup(STDOUT_FILENO);

    dup2(_clientfd, STDOUT_FILENO);

    printf("****************************************\n"
	       "** Welcome to the information server. **\n" 
	       "****************************************\n");

    // printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaa\n");
    printf("*** User '(no name)' entered from %s:%d. ***\n", ipv4, ntohs(clientInfo.sin_port));
    // printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaa\n");
    send(_clientfd, "% ", strlen("% "), 0);
    // printf("bbbbbbbbbbbbbbbbbbbbbbbbbbbbb\n");
    
    // printf("clientSize is: %d\n", serv_table_ptr->clientSize);
    for ( int i=0;i<serv_table_ptr->clientSize;i++ )
    {
        // printf("clientfd is: %d\n", _clientfd);
        // printf("clientfd[i] is: %d\n", serv_table_ptr->clientfds[i]);
        if ( serv_table_ptr->clientfds[i] == _clientfd ) continue;
        if ( serv_table_ptr->clientfds[i] == 0 ) continue;

        dup2(serv_table_ptr->clientfds[i], STDOUT_FILENO);

        printf("*** User '(no name)' entered from %s:%d. ***\n", ipv4, ntohs(clientInfo.sin_port));
    }

    dup2(prev_fd, STDOUT_FILENO);
    close(prev_fd);

}

void setclientenv()
{
    int index = get_client_index(clientfd);

    if ( index < 0 ) return;

    

    for ( int i=0;i<serv_table_ptr->clientEnv[index].env_num;i++ )
    {
        char* env;
        char buffer[1000] = "";

        strcpy(buffer, serv_table_ptr->clientEnv[index].env_val[i]);
        env = getenv(serv_table_ptr->clientEnv[index].env_name[i]);
        strcpy(serv_table_ptr->clientEnv[index].env_val[i], env);
        setenv(serv_table_ptr->clientEnv[index].env_name[i], buffer, 1);
    }
}

void EXE_server()
{
    // printf("SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS\n");
    int bufferlen = 16000;
    
    char* buffer = (char*)calloc(bufferlen, sizeof(char));

    if ( clientfd != 0 )
    {
        // printf("RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR\n");
        recv(clientfd, buffer, sizeof(char)*bufferlen, 0);
        // printf("rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr\n");
        char tmp_buffer[16000] = "";

        strcpy(tmp_buffer, buffer);
        string tmp_str(tmp_buffer);

        parsed_command cmd(tmp_str);

        buffer[strcspn(buffer, "\r\n")] = 0;

        if ( cmd.processes.size() != 0 ) 
        {
            execute_command(cmd, buffer); //debug
        }

        send(clientfd, "% ", strlen("% "), 0);

        memset(buffer, 0, bufferlen);
    }

    free(buffer);
}

void print_user_pipe()
{
    printf("------------------pipe list--------------------------\n");

	printf("client:%d\n", get_client_index(clientfd) + 1);
    

    for ( int i=0;i< user_Ptable_ptr->pipe_num;i++ )
    {
        printf("pipe_id:%d,  %d to %d, fd[0]:%d, fd[1]:%d\n", i, user_Ptable_ptr->inIndex[i] + 1, user_Ptable_ptr->outIndex[i] + 1, user_Ptable_ptr->inPipe[i], user_Ptable_ptr->outPipe[i]);
    }
}

void EXE_exit_server()
{
    int index = get_client_index(clientfd);

    setclientenv();
    

    serv_table_ptr->clientfds[index] = 0;
    serv_table_ptr->clientEnv[index].env_num = 0;
    
    
    for ( int i=0;i<user_Ptable_ptr->pipe_num;i++ )
    {
        if ( user_Ptable_ptr->inIndex[i] == index || user_Ptable_ptr->outIndex[i] == index )
        {
            if ( user_Ptable_ptr->inPipe[i] != 0 ) close(user_Ptable_ptr->inPipe[i]);
            if ( user_Ptable_ptr->outPipe[i] != 0 ) close(user_Ptable_ptr->outPipe[i]);

            user_Ptable_ptr->inIndex[i] = 0;
            user_Ptable_ptr->outIndex[i] = 0;
            user_Ptable_ptr->inPipe[i] = 0;
            user_Ptable_ptr->outPipe[i] = 0;
        }
    }

    msg->flag = 6;
    *speaker_idx = index;

    for ( int i=0;i<serv_table_ptr->clientSize;i++ )
    {
        if ( serv_table_ptr->clientfds[i] != 0 )
        {
            int pid = serv_table_ptr->p_table[i];
            
            kill(pid, SIGUSR1);
            // printf("*** User '%s' left. ***\n", serv_table_ptr->clientName[index]);
        }
    }

    exit(0);
}

void set_server_env(char* envName, char* envValue)
{
    int index = get_client_index(clientfd);
    int label = 0;
    

    for ( int i=0;i<serv_table_ptr->clientEnv[index].env_num;i++ )
    {
        if ( strcmp(serv_table_ptr->clientEnv[index].env_name[i], envName) == 0 ) label = 1;
    }

    if ( label == 0 )
    {
        char* env;

        strcpy(serv_table_ptr->clientEnv[index].env_name[serv_table_ptr->clientEnv[index].env_num], envName);
        env = getenv(envName);
        strcpy(serv_table_ptr->clientEnv[index].env_val[serv_table_ptr->clientEnv[index].env_num], env);
        serv_table_ptr->clientEnv[index].env_num++;
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
    // printf("pppp index1 is %d\nindex2 is %d\n", *speaker_idx, *listener_idx);
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
    // printf("uuuu index1 is %d\nindex2 is %d\n", *speaker_idx, *listener_idx);
    execute_process(p,"","",np_fd,get_write_fifo(client_fd_id), read_fd, head, false);
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
    // printf("cccc index1 is %d\nindex2 is %d\n", *speaker_idx, *listener_idx);
    
    if (np_fd > 0)
    {
        EXE_np(np_fd);
    }
    if ( is_user_pipe)
    {
        msg->flag = 5;
        kill(serv_table_ptr->p_table[*listener_idx], SIGUSR1);


        int tmp_fd = open(msg->file_path, O_WRONLY);
        msg->rwfd[1] = tmp_fd;
        dup2(tmp_fd, STDOUT_FILENO);
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
        Wait(pid);
    if ( is_user_pipe )
    {
        close(msg->rwfd[1]);
    }
}

void EXE_exit()
{
    if ( clientfd > 0 )
    {
        free_pipe_table(&client_pipe_table);
        close(clientfd);
    }

    EXE_exit_server();
}

void EXE_setenv(vector<string> proc)
{
    if (proc[1].length() == 0 || proc[2].length() == 0)
        return;
    set_server_env(&proc[1][0], &proc[2][0]);
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

void kill_tell()
{
    // printf("TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT\n");
    int index = *speaker_idx;
    int tmp_len = msg->len;
    char tmp_char[16000];
    strcpy(tmp_char, msg->full_msg);

    printf("*** %s told you ***: %s\n", serv_table_ptr->clientName[index], tmp_char + tmp_len);
}

void kill_yell()
{
    // printf("YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY\n");

    int index = *speaker_idx;
    int tmp_len = msg->len;
    char tmp_char[15000];
    // printf("full msg is %s\n", msg->full_msg);
    strcpy(tmp_char, msg->full_msg);
    // printf("tmp_char is %s\n", tmp_char);
    printf("*** %s yelled ***: %s\n", serv_table_ptr->clientName[index], tmp_char + tmp_len);
}

void kill_name()
{
    int port = 0;
    int index = *speaker_idx;
    char ip[20];

    struct sockaddr_in info = get_client_info(index);

    inet_ntop(AF_INET, &info.sin_addr, ip, sizeof(struct sockaddr));
    port = ntohs(info.sin_port);

    printf("*** User from %s:%d is named '%s'. ***\n", ip, port, serv_table_ptr->clientName[index]);
}

void kill_Upipe()
{
    int index1 = *speaker_idx;
    int index2 = *listener_idx;

    char command[15000];
    strcpy(command, msg->full_msg);

    printf("*** %s (#%d) just piped '%s' to %s (#%d) ***\n", serv_table_ptr->clientName[index1], index1 + 1, command, serv_table_ptr->clientName[index2], index2 + 1);
}

void kill_Urecv()
{
    int index1 = *speaker_idx;
    int index2 = *listener_idx;

    char command[15000];
    strcpy(command, msg->full_msg);

    printf("*** %s (#%d) just received from %s (#%d) by '%s' ***\n", serv_table_ptr->clientName[index2], index2 + 1, serv_table_ptr->clientName[index1], index1 + 1, command);
}

void kill_open()
{
    int index1 = *speaker_idx;
    int index2 = *listener_idx;

    string folder = "user_pipe/";
    // printf("index1 is %d\nindex2 is %d\n", index1, index2);
    int num = index1*60 + index2 + 1;
    string fifo_name = to_string(num);

    char file_path[20];
    strcpy(file_path, &folder[0]);
    strcat(file_path, &fifo_name[0]);
    // printf("path is %s\n", file_path);
    int rfd = open(file_path, O_RDONLY|O_NONBLOCK);
}

void kill_exit()
{
    printf("*** User '%s' left. ***\n", serv_table_ptr->clientName[*speaker_idx]);
}

void EXE_name ( vector<string> p)
{
    if ( p[1].length() == 0 ) return;

    if ( set_client_name(p[1]) == 0 )
    {
        printf("*** User '%s' already exists. ***\n", p[1].c_str());
        return;
    }

    int port = 0;
    int* client_fds = get_all_fds();
    int index = get_client_index(clientfd);
    char ip[20];

    struct sockaddr_in info = get_client_info(index);

    inet_ntop(AF_INET, &info.sin_addr, ip, sizeof(struct sockaddr));
    port = ntohs(info.sin_port);    msg->flag = 2;

    

    *speaker_idx = index;

    for ( int i=0;i<serv_table_ptr->clientSize;i++ )
    {
        if ( client_fds[i] == 0 ) continue;
        
        kill(serv_table_ptr->p_table[i], SIGUSR1);
    }

    dup2(clientfd, STDOUT_FILENO);
}

void EXE_who ( vector<string> p )
{
    printf("<ID>    <nickname>  <IP:port>   <indicate me>\n");

    int port = 0;
    int* client_fds = get_all_fds();
    struct sockaddr_in clientinfo;
    string tmp = "";
    char* client_name = &tmp[0];
    char ip[20];
    

    for ( int i=0;i<serv_table_ptr->clientSize;i++ )
    {
        if ( client_fds[i] == 0 ) continue;

        client_name = serv_table_ptr->clientName[i];
        clientinfo = get_client_info(i);
        inet_ntop(AF_INET, &clientinfo.sin_addr, ip, sizeof(struct sockaddr) );
        port = ntohs(clientinfo.sin_port);

        if ( client_fds[i] == clientfd )
        {
            printf("%d  %s  %s:%d   <-me\n", i+1, client_name, ip, port);
        }
        else
        {
            printf("%d  %s  %s:%d\n", i+1, client_name, ip, port);
        }

    }
}

void EXE_tell ( vector<string> p )
{
    if ( p[1] == "" || p[2] == "" ) return;

    int index =  stoi(p[1]) -1;
    int len = strspn(full_command, "tell");
    int* all_fds = get_all_fds();
    char buffer[30] = "tell";
    string tmp = " ";
    char* space = new char[tmp.length()+1];
    strcpy(space, tmp.c_str());

    for ( int i=len;i<strlen(full_command);i++ )
    {
        if ( full_command[i] != ' ') break;
        len = i+1;
        strcat(buffer, space);
    }

    strcat(buffer, p[1].c_str());
    len = strspn(full_command, buffer);

    for ( int i = len;i<strlen(full_command);i++ )
    {
        if ( full_command[i] != ' ') break;
        len = i+1;
    }

    if ( all_fds[index] == 0 )
    {
        printf("*** Error: user #%d does not exist yet. ***\n", atoi(p[1].c_str()));
    }
    else
    {
        msg->flag = 0;

        pid_t pid = serv_table_ptr->p_table[index];
        // printf("pid is: %d\n", pid);
        index = get_client_index(clientfd);
        // printf("index is: %d\n", index);
        *speaker_idx = index;
        strcpy(msg->full_msg, full_command);
        // printf("msg is: %s\n", msg->full_msg);
        msg->len = len;
        // printf("msg len is: %d\n", msg->len);

        kill(pid, SIGUSR1);

        dup2(clientfd, STDOUT_FILENO);
    }
}

void EXE_yell ( vector<string> p )
{
    if ( p[1] == "" ) return;

    int index = get_client_index(clientfd);
    int len = strspn(full_command, "yell");
    int* all_fds = get_all_fds();
    for ( int i=len;i<strlen(full_command); i++ )
    {
        if ( full_command[i] != ' ') break;
        len = i+1;
    }

    msg->flag = 1;
    *speaker_idx = index;
    // printf("speaker_idx is: %d\n", *speaker_idx);
    strcpy(msg->full_msg, full_command);
    // printf("full msg is: %s\n", msg->full_msg);
    msg->len = len;
    // printf("len is: %d\n", len);
    // printf("msg len is: %d\n", msg->len);

    for ( int i=0;i<serv_table_ptr->clientSize;i++ )
    {
        if ( all_fds[i] == 0 ) continue;

        if ( all_fds[i] == clientfd )
        {
            printf("*** %s yelled ***: %s\n", serv_table_ptr->clientName[index], full_command + len);
        }
        else
        {
            // printf("pid is: %d\n", serv_table_ptr->p_table[i]);
            kill(serv_table_ptr->p_table[i], SIGUSR1);
        }
        
    }

    dup2(clientfd, STDOUT_FILENO);
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
    

    for ( int i=0;i<=pool_ptr->size;i++ )
    {
        if ( pool_ptr->value[i] == 0 )
        {
            pool_ptr->value[i] = pid;

            if ( i == pool_ptr->size )
            {
                pool_ptr->size++;
            }

            return;
        }
    }
}

void ClearPid ( int pid )
{
    

    for ( int i=0;i<pool_ptr->size;i++ )
    {
        if ( pool_ptr->value[i] == 0 ) continue;

        if ( pool_ptr->value[i] == pid )
        {
            pool_ptr->value[i] = 0;
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

        for ( int i=0;i<pool_ptr->size;i++ )
        {
            waitPID = waitpid(pool_ptr->value[i], &status, WNOHANG);

            if ( waitPID == pool_ptr->value[i] )
            {
                ClearPid(waitPID);
            }

            if ( waitPID == pid ) return;

            if ( pid == pool_ptr->value[i] ) label = 1;
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
    else if ( p[0].compare("name") == 0 )   //
    {
        EXE_name(p);
        return 1;
    }
    else if ( p[0].compare("who") == 0 )
    {
        EXE_who(p);
        return 1;
    }
    else if ( p[0].compare("tell") == 0 )
    {
        EXE_tell(p);
        return 1;
    }
    else if ( p[0].compare("yell") == 0 )
    {
        EXE_yell(p);
        return 1;
    }

    return 0;
}

int set_client_name ( string name )
{
    char* tmp_name = new char[name.length()+1];
    strcpy(tmp_name,name.c_str());
    

    for ( int i=0;i<serv_table_ptr->clientSize;i++ )
    {
        if ( strcmp(serv_table_ptr->clientName[i], tmp_name) == 0 ) return 0;
    }

    strcpy(serv_table_ptr->clientName[get_client_index(clientfd)], tmp_name);
    delete []tmp_name;    
    return 1;
}

int add_fifo( int  pipe_id)
{
    if ( serv_table_ptr->clientfds[pipe_id] == 0 ) return -1;

    int index = get_client_index(clientfd);

    if ( user_Ptable_ptr->fifo_exist[index][pipe_id] ) return 0;

    return 1;
}

    // 0-0 = 1 i*60+j+1
int get_read_fifo ( int pipe_id, int* readfd)
{
    if ( serv_table_ptr->clientfds[pipe_id] == 0 ) return -1;

    int index = get_client_index(clientfd);
    if ( user_Ptable_ptr->fifo_exist[pipe_id][index] )
    {
        string folder = "user_pipe/";
        int num = pipe_id*60 + index + 1;
        string fifo_name = to_string(num);

        char file_path[20];
        strcpy(file_path, &folder[0]);
        strcat(file_path, &fifo_name[0]);
        // printf("path is %s\n", file_path);
        *readfd = open(file_path, O_RDONLY|O_NONBLOCK);// open(fn, O_RDONLY|O_NONBLOCK)
        
        msg->rwfd[0] = *readfd;
        dup2(*readfd, STDIN_FILENO);
        return 1;
    }
    return 0;
}

int* get_write_fifo ( int pipe_id)
{
    int* fifo_fd = (int*)malloc(sizeof(int)*2);
    int index = get_client_index(clientfd);

    string folder = "user_pipe/";
    int num = index*60 + pipe_id + 1;
    string fifo_name = to_string(num);

    char file_path[100];
    strcpy(file_path, &folder[0]);
    strcat(file_path, &fifo_name[0]);
    // printf("paht is %s\n", file_path);
    strcpy(msg->file_path, file_path);
    fifo_fd[1] = open(file_path, O_WRONLY);// open(fn, O_RDONLY|O_NONBLOCK)
    //fifo_fd[1] = open(file_path, O_RDONLY|O_NONBLOCK);// open(fn, O_RDONLY|O_NONBLOCK)

    // msg->file_path = file_path;
    // printf("fd is %d\n", fifo_fd[1]);
    // dup2(fifo_fd[1], STDOUT_FILENO);
    return fifo_fd;
}

int* get_all_fds()
{
    
	return serv_table_ptr->clientfds;
}

int get_client_index ( int _clientfd )
{
    

    for ( int i =0;i<serv_table_ptr->clientSize;i++)
    {
        if ( serv_table_ptr->clientfds[i] == _clientfd ) return i;
    }

    return -1;
}

int server(int port)
{
    // printf("SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS\n");
    int sockfd = 0;
    int Clientsockfd = 0;
    int opt = 1;

    if (!init)
    {
        for ( int i =0;i<60;i++ )
        {
            // printf("IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII\n");
            serv_table_ptr->clientfds[i] = 0;
            // printf("IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII\n");
        }
        init = true;
    }    

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    
    printf("Start server at port:%d\n", port);
    
    if ( sockfd == -1 ) printf("fail to create sockt\n");

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
    bind(sockfd, (struct sockaddr*) &serverInfo, sizeof(serverInfo));    
    listen(sockfd, 60);
    


    while(1)
    {
        // printf("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
        Clientsockfd = accept(sockfd, (struct sockaddr*) &clientInfo, &addrlen);
        // printf("Clientsockfd is %d\n", Clientsockfd);
        pid_t pid = fork();

        if ( pid != 0 ) 
        {
            // close(Clientsockfd);
            continue;
        }
        
        if ( Clientsockfd < 0 ) printf("accept error\n");
        else
        {
            printf("-----------------------------------------------------------\n");
            printf("                        New user login                     \n");	
            printf("-----------------------------------------------------------\n");
            send_login_message(Clientsockfd, clientInfo);

            for ( int i = 0;i<= serv_table_ptr->clientSize;i++ )
            {
                if ( serv_table_ptr->clientfds[i] == 0 )
                {
                    serv_table_ptr->clientfds[i] = Clientsockfd;
                    serv_table_ptr->clientInfo[i] = clientInfo;
                    strcpy(serv_table_ptr->clientName[i], "(no name)");
                    serv_table_ptr->p_table[i] = getpid();

                    if ( i == serv_table_ptr->clientSize ) serv_table_ptr->clientSize++;
                    break;
                }
            }

        }
        
        clientfd = Clientsockfd;
        setclientenv();

        dup2(Clientsockfd, STDERR_FILENO);
        dup2(Clientsockfd, STDOUT_FILENO);

        while(1)
        {
            EXE_server();
        }

        setclientenv();        

        print_user_pipe();
        exit(0);
    } 

    return 0;
}

struct sockaddr_in get_client_info ( int client_id )
{
    

    return serv_table_ptr->clientInfo[client_id];
}

void handler(int sig)
{
    // printf("HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH\n");
    switch (msg->flag)
    {
        case 0:
            kill_tell();
            break;
        case 1:
            kill_yell();
            break;
        case 2:
            kill_name();
            break;
        case 3:
            kill_Upipe();
            break;
        case 4:
            kill_Urecv();
            break;
        case 5:
            kill_open();
            break;
        case 6:
            kill_exit();
        default:
            break;
    }
    // if( sig ==SIGUSR1 )
    // {
    //     broadcost(listener_idx, msg->full_msg);
    // }
    // else ( sig ==SIGUSR2 )
    // {

    // }

    return;
}

void init_fifo_table (user_pipe_table* Ptable)
{
    for( int i =0; i<60; i++)
    {
        for( int j=0; j<60; j++)
        {
            Ptable->fifo_exist[i][j] = false;
        }
    }
}

void fifo_create()
{
    string folder = "user_pipe/";

    for ( int i=0;i<3600;i++ )
    {
        int num = i+1;
        string fifo_name = to_string(num);

        char file_path[20];
        strcpy(file_path, &folder[0]);
        strcat(file_path, &fifo_name[0]);
        remove(file_path);
        int val = mkfifo(file_path, S_IFIFO|0666);
    }
}

int main(int argc, char **argv, char **envp)
{
    int port = 8888;

    int tmp_1 = shmget(IPC_PRIVATE, sizeof(serv_table), IPC_CREAT | 0666);
    int tmp_2 = shmget(IPC_PRIVATE, sizeof(user_pipe_table), IPC_CREAT| 0666);
    int tmp_3 = shmget(IPC_PRIVATE, sizeof(pool), IPC_CREAT| 0666);
    int tmp_5 = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT| 0666);
    int tmp_6 = shmget(IPC_PRIVATE, sizeof(msg_info), IPC_CREAT| 0666);
    int tmp_7 = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT| 0666);//listener_idx

    serv_table_ptr = (serv_table *) shmat(tmp_1, NULL, 0);
    user_Ptable_ptr = (user_pipe_table *) shmat(tmp_2, NULL, 0);
    pool_ptr = (pool *) shmat(tmp_3, NULL, 0);
    speaker_idx = (int*) shmat(tmp_5, NULL, 0);
    msg = (msg_info*) shmat(tmp_6, NULL, 0);
    listener_idx = (int*) shmat(tmp_7,NULL, 0);

    serv_table_ptr->clientMax = 60;
    serv_table_ptr->clientSize = 0;
    user_Ptable_ptr->pipe_num = 0;
    pool_ptr->size = 0;

    init_fifo_table(user_Ptable_ptr);
    fifo_create();
    signal(SIGUSR1, handler);
    if (argc > 1)
    {
        port = atoi(argv[1]);
    }
    setenv("PATH", "bin:.", 1);

    execute_loop(port);

    shmdt(serv_table_ptr);
    shmdt(user_Ptable_ptr);
    shmdt(pool_ptr);
    shmdt(speaker_idx);
    shmdt(msg);
    shmdt(listener_idx);

    shmctl(tmp_1, IPC_RMID, NULL);
    shmctl(tmp_2, IPC_RMID, NULL);
    shmctl(tmp_3, IPC_RMID, NULL);
    shmctl(tmp_5, IPC_RMID, NULL);
    shmctl(tmp_6, IPC_RMID, NULL);
    shmctl(tmp_7, IPC_RMID, NULL);

    return 0;
}