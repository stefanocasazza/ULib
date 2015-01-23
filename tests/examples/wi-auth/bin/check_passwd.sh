#/bin/bash

# check_passwd.sh

			  # egrep "[ABCDEFGHIJKLMNOPQRSTUVXYZ]" | \
b=`echo $1 | egrep "^.{8,255}" | \
             egrep "[abcdefghijklmnopqrstuvxyz"] | \
             egrep "[0-9]"`

#if the result string is empty, one of the conditions has failed

if [ -z $b ]
  then
    echo "Conditions do not match"
  else
    echo "Conditions match"
fi
