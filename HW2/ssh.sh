#!/usr/bin/bash

####################################################################
##                 scp and ssh to remote quickly                  ##
##       only need to check your ssh can access directly          ##
##      If not, please use "ssh-copy-id username@server_host"     ##
##                    Written By Jason, CHEN                      ##
####################################################################

# If you only want to copy one file to server, please use
# scp /your_file_path username@server:your_server's_file_path

# If you want to copy all files to server, please use
scp -r /home/jason/311552039_np_project2/* zhangtw528491@nplinux11.cs.nctu.edu.tw:/net/gcs/111/311552039/np_project2
# scp -r /home/jason/np_project2_demo_sample zhangtw528491@nplinux11.cs.nctu.edu.tw:/net/gcs/111/311552039/np_project2

# ex: scp -r /home/sheng/np_project/np_project2/31155xxxx_np_project2 username@nplinux11.cs.nctu.edu.tw:/u/gcs/111/31155xxxx/

echo ""
echo "======= Login Remote Access ======="
echo ""

# Below will let you use "make" in server and cd into your file path and stay.
ssh -t zhangtw528491@nplinux11.cs.nctu.edu.tw "cd np_project2; make; /bin/bash"
