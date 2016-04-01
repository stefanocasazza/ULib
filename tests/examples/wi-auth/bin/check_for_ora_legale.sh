#!/bin/sh

TCP_PID=`cat /var/run/userver_tcp.pid`
SSL_PID=`cat /var/run/userver_ssl.pid`

kill -WINCH $TCP_PID $SSL_PID
