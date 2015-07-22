#!/bin/bash

ip=192.168.151.111
port=8889

cnt=0
while true
do
	((cnt++))
	echo "connect num: $cnt"
	./client $ip $port "hi nice to meet you"
	
	if [ $? -ne 0 ];then
		break
	fi
done
