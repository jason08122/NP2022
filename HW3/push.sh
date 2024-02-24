#!/bin/bash

echo "git push..."
function getmsg(){
	read -p "please input the commit message: " message

	if [ ! -n "$message" ];then
		echo "need to input the commit message... "
		getmsg
	else
		git commit -m "$message"
	fi
}

git add .
getmsg
git push origin master
echo "git push success... !"
