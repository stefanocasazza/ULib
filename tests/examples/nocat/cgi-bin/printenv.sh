#!/bin/sh

# tcpdump -tqni wlan0 host 192.168.119.26

# printenv -- demo CGI program which just prints its environment

echo -e 'Content-Type: text/html; charset=utf8\r\n\r'
echo '<pre>'
env
echo '</pre>'
