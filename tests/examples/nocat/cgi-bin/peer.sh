#!/bin/sh

# peer.sh

# set -x

REMOTE_MAC=`arp | grep $REMOTE_ADDR 2>/dev/null`

if [ -z "$ROW" ]; then
	REMOTE_MAC=00:00:00:00:00:00
fi

echo "REMOTE_MAC=`echo $REMOTE_MAC | cut -d ' ' -f4`"
