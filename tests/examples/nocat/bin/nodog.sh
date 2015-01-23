#!/bin/sh

test "$1" = -h && { echo usage: "$0 [<start> [<stop> [<sleep>]]]" ; exit 1 ; }

start=${1:-1}
 stop=${2:-56}
sleep=${3:-60}

test -f ~/lg.env && . ~/lg.env || . /etc/profile > /dev/null

f() { local start=${1:-1} stop=${2:-10}
	
	i=$start ; while test $i -le $stop ; do
		echo -n "Captive_$i 10.8.0.$i "
		ping -qc1 -W3 10.8.0.$i > /dev/null && {
			echo | telnet 10.8.0.$i 5280 2> /dev/null &&
				echo nodog OK | color -green . ||
				echo nodog DOWN | color -yellow .
			} ||
				echo not responding | color -red .
		i=`expr $i + 1`
	done
}

repeat -C -d -n -s $sleep f $start $stop
