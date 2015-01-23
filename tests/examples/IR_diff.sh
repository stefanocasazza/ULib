#!/bin/sh

sort out/query.out > query.out1
sort  ok/query.ok	 > query.ok1

mv query.out1		out/query.out
mv query.ok1		 ok/query.ok

gvimdiff out/query.out ok/query.ok
