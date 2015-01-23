#!/bin/bash

source /srv/wifi-portal-firenze/etc/environment.conf

ping -c 2 $RSYNC_HOST > /dev/null 2>/dev/null &&
(	rsync -az --delete \
	      $DOCUMENT_ROOT/counter \
	      $DOCUMENT_ROOT/login \
	      $DOCUMENT_ROOT/registration \
	      $DOCUMENT_ROOT/request \
	      $DOCUMENT_ROOT/stat \
	      $RSYNC_HOST:$DOCUMENT_ROOT/
	rsync -az \
	      $FILE_LOG \
	      $RSYNC_HOST:/var/log
)
