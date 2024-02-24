# :pencil: HW 1 - NPShell
    
## Introduction
    
The project include develope a shell named **npshell**, support the following features.

1. Execution of commands
2. Ordinary Pipe
3. Numbered Pipe
4. File Redirection

---

## Execution of commands

- **NPShell Behavior**
The npshell parses the inputs and executes commands

    ```typescript
    % ls
    bin npshell test1.txt test2.txt test1.html 
    ```
- **Built-in Commands**
    - **setenv**
    ```typescript
    % setenv PATH bin        # set PATH to bin
    % setenv PATH bin:npbin  # set PATH to bin:npbin
    ```
    
    - **printenv**
    ```typescript
    % printenv LANG
    en_US.UTF-8
    % printenv VAR1  # show nothing if the variable does not exist
    % setenv VAR1 test
    % printenv VAR1
    test
    ```
    
    - **exit**
    Terminate npshell.
- **Unknown Command**

    Print the error message to **STDERR** with format: 
    **Unknown command: [command]**.

    ```typescript
    % aaa
    Unknown command: [aaa].
    ```
---

## Ordinary Pipe

```typescript
#The output of the command "ls" acts as the input of the command "cat" 
% ls | cat
bin
npshell
test.html
```



## Numbered Pipe

**|N** means the **STDOUT** of the left hand side command will be piped to **the first command of the next N-th line**, where 1 <= N <= 100

```typescript
% ls |2
% set PATH bin
% cat      # The output of "ls" acts as the input of "cat"
bin
npshell
test.html
```

## File Redirection
**Standard output redirection (cmd > file)**, which means the output of the command will be written to files.

```typescript
# The output of "ls" is redirected to file "hello.txt"
% ls > hello.txt
% cat hellow.txt
bin
npshell
test.html
```
<br>

# :pencil: HW 2 - Remote Working Ground Server

## Introduction

The project include 3 kinds of servers:

1. Concurrent **connection-oriented** server, allows one clientconnect to it.
2. **Chat-like** server. In the system, users can communicate with other users. Using **singgle-process concurrent** paradigm to design the server.
3. **Chat-like** server, but use **concurrent connection-oriented** paradigm with **shared memory** and **FIFO**.

## Functionality

1. Pipe between different users. Broadcast messgae whenever a user pipe is used.
2. Broadcast message of login/logout information.
3. New built-in commands:

    - **who**: show information of all users.
    - **tell**: send a message to another user.
    - **yell**: send a message to all users.
    - **name**: change your name. 
4. All commands in **HW 1**

## New built-in commands

- **who**
    ```typescript
    % who
    <ID> <nickname> <IP:port> <indicate me>
    1 IamStudent 140.113.215.62:1201 <-me
    2 (no name) 140.113.215.63:1013
    3 student3 140.113.215.62:1201
    ```

- **tell**  {user id} {message}
    ```typescript
    # Assume my name is ’IamStudent’.
    # [terminal of mine]
    % tell 3 Hello World.
        
    # If user 3 exists,
    # [terminal of user id 3]
    % *** IamStudent told you ***: Hello World.
    
    # If user 3 doesn’t exist,    
    # [terminal of mine]
    % tell 3 Hello World.
    *** Error: user #3 does not exist yet. ***
    ```

- **yell**  {message}
    ```typescript
    # Assume my name is ’IamStudent’.
    # [terminal of mine]
    % yell Good morning everyone.
    *** IamStudent yelled ***: Good morning everyone.
    
    # [terminal of all other users]
    % *** IamStudent yelled ***: Good morning everyone.
    ```
- **name** {new name}
    ```typescript
    # [terminal of mine]
    % name Mike
    *** User from 140.113.215.62:1201 is named ’Mike’. ***
    
    # [terminal of all other users]
    % *** User from 140.113.215.62:1201 is named ’Mike’. ***
    
    # If Mike is on-line, and I want to change name to Mike, this name change will fail.
    # [terminal of mine]
    % name Mike
    *** User ’Mike’ already exists. ***
    ```

    
# :pencil: HW 3 - HTTP Server and CGI Programs
    
## Introduction
The project is divided into two parts. 

- The first part of the project is to write a **simple HTTP server** called http_server and a CGI program **console.cgi**. Also, we'll be Using **Boost.Asio** library to accomplish this project.

- For the second part, you are asked to provides the same functionality as **part 1**, but with some rules slightly

    **Differs**:
    1. Implement one program, **cgi_server.exe**, which is a combination of http_server, panel.cgi, and console.cgi.
    2. Your program should run on **Windows 10**.

## Part 1

### http_server
    
1. In this project, the URI of HTTP requests will always be in the form of /${cgi name}.cgi
(e.g., /panel.cgi, /console.cgi, /printenv.cgi), and we will only test for the HTTP GET method.
2. Your **http server** should parse the HTTP headers and **follow the CGI procedure** (fork, set
    environment variables, dup, exec) to execute the specified CGI program.
3. The following environment variables are required to set:
    * REQUEST METHOD
    * REQUEST URI
    * QUERY STRING
    * SERVER PROTOCOL
    * HTTP HOST
    * SERVER ADDR
    * SERVER PORT
    * REMOTE ADDR
    * REMOTE PORT
    
### console.cgi
The **console.cgi** should parse the connection information (e.g. host, port, file) from the environment
variable **QUERY STRING**, which is set by your HTTP server (see section 2.1).

For example, if **QUERY STRING** is:

```typescript
h0=nplinux1.cs.nctu.edu.tw&p0=1234&f0=t1.txt&h1=nplinux2.cs.nctu.edu.tw&
p1=5678&f1=t2.txt&h2=&p2=&f2=&h3=&p3=&f3=&h4=&p4=&f4=
```

It should be understood as:

```typescript
h0=nplinux1.cs.nctu.edu.tw # the hostname of the 1st server
p0=1234 # the port of the 1st server
f0=t1.txt # the file to open

h1=nplinux2.cs.nctu.edu.tw # the hostname of the 2nd server
p1=5678 # the port of the 2nd server
f1=t2.txt # the file to open

h2= # no 3rd server, so this field is empty
p2= # no 3rd server, so this field is empty
f2= # no 3rd server, so this field is empty

h3= # no 4th server, so this field is empty
p3= # no 4th server, so this field is empty
f3= # no 4th server, so this field is empty

h4= # no 5th server, so this field is empty
p4= # no 5th server, so this field is empty
f4= # no 5th server, so this field is empty
```

### panel.cgi

This CGI program generates the form in the **web** page. It detects all files in the directory test case/ and display them in the selection menu.



## Part 2

### cgi_server.exe

1. The **cgi_server.exe** accepts TCP connections and parse the HTTP requests (as **http_server** does),
and we will only test for the HTTP GET method.

2. You don’t need to fork() and exec() since it’s relatively hard to do it on Windows. Simply parse the request and do the specific job within the same process. We guarantee that in this part the URI of HTTP requests will be ”/panel.cgi” or ”/console.cgi” plus a query string:

    * If it is **/panel.cgi**, Display the panel form just like panel.cgi in part 1. This time, you can hard code the input file menu (t1.txt ∼ t5.txt).

    * If it is **/console.cgi?h0=...**, Connect to remote servers specified by the query string. Note that the behaviors **MUST** be the same as part 1 **in the user’s point of view** (though the procedure is different in this part).


<br>    

# :pencil: HW 4 

