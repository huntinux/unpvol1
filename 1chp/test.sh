#!/bin/bash

# get ip 
ipaddr=$(ifconfig | grep "inet addr" | head -1 | cut -d: -f 2 | cut -d' ' -f 1)

# start server
#./daytimetcpsrv &

# client
./daytimetcpcli $ipaddr

# kill srv
sleep 2  # �ȴ�2�룬��֤ͨ�����
srvpid=$(ps aux | grep daytimetcpsrv | grep -v grep | awk -F' '  '{print $2}')
for pid in $srvpid
do
	echo "Kill $pid"
	kill $pid
done
