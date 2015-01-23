#!/bin/sh

## web_server_multiclient.diff.sh

grep cached out/web_server_multiclient.out | cut -d':' -f4 | sort > /tmp/a
grep cached  ok/web_server_multiclient.ok  | cut -d':' -f4 | sort > /tmp/b

gvimdiff /tmp/a /tmp/b &

sleep 10
rm -f	/tmp/a /tmp/b
