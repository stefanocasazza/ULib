#!/bin/sh

awk '/^dn:/{print $0;printf "changeType: delete\n\n"; }' <$1
