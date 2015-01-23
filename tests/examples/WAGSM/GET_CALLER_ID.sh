#/bin/bash

# GET_CALLER_ID.sh

#set -x

reset
echo -n  > ./minicom.log
minicom -C ./minicom.log -S ./PiazzeAperte
reset
