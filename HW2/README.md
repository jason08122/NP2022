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