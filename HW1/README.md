# :pencil: HW 1 - NPShell
    
## Introduction
    
The project include develope a shell named **npshell**, support the following features.

1. Execution of commands
2. Ordinary Pipe
3. Numbered Pipe
4. File Redirection



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