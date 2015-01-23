#!/bin/sh

# -------------------------------------
# URI_PATH      /my/stream
# CONTENT_TYPE  text/plain
# -------------------------------------
while `true`; do date; sleep 1; done
# -------------------------------------

# Turn a sequence of images into video:
# ffmpeg -f image2 -i image%d.jpg video.mpg

# Turn a video into a sequence of images:
# ffmpeg -i video.mpg image%03d.jpg

# -------------------------------------
# URI_PATH      /mjpeg
# CONTENT_TYPE  "multipart/x-mixed-replace; boundary=++++++++"
# -------------------------------------
count=0
BOUNDARY=++++++++

for i in `ls mjpeg/*`; do
	FILE[$count]=$i
   SIZE[$count]=`ls -l $i | awk '{ print $5 }'`
   ((count++))
done

#printf "HTTP/1.1 200 OK\r\n"
#printf "Server: ULib/1.0\r\n"
#printf "Date: Wed, 19 May 2010 17:16:24 GMT\r\n"
#printf "Content-Type: multipart/x-mixed-replace; boundary=%s\r\n\r\n" ${BOUNDARY}

while true; do
	n=0
	while [ $n -lt $count ]; do
		printf "\r\n--%s\r\n" ${BOUNDARY}
		printf "Content-type: image/jpeg\r\n"
		printf "Content-length: %d\r\n\r\n" ${SIZE[$n]}
		cat ${FILE[$n]}
		((n++))
	done
done

#printf "\r\n\r\n--%s--\r\n" ${BOUNDARY}
# -------------------------------------
