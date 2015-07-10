#!/bin/sh

#set -x

for i in *.usp; do
#	rm   $(basename $i .usp).cpp
 	make $(basename $i .usp).cpp
done
