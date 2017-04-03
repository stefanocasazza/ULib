#!/bin/sh

#IF=eth0
 IF=enp0s20u1

echo -e "\r"`date`

grep "$IF" /proc/net/dev
