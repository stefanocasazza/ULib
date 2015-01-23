#!/bin/sh

# 32000|18/04/05 17:58:06|Startup   lt-lrp_session
# 32015|18/04/05 17:58:06|10.10.1.44|webservices-amount|WebServices|apply|true
# 32014|18/04/05 17:58:07|10.30.1.2|webservices-amount|WebServices|apply|false
# 32016|18/04/05 17:58:07|10.30.1.2|webservices-rate|WebServices|apply|false
# 32000|18/04/05 17:58:07|Shutdown  lt-lrp_session

test $# -ge 1 && cd $1
test $# -ge 2 && value=${2:-*}

{
	 zcat *.log.*.gz 2>/dev/null
	  cat *.log

} | gawk -F '|' '

	BEGIN {
		sortfs="\t"
	}

	$NF ~ "^Startup" {
		split($2, a, " ")
		data=a[1]
		time=a[2]

		split(data, a, "/")
		sessiondd=a[1]
		sessionmm=a[2]
		sessionyy=a[3]

		split(time, a, ":")
		sessionH=a[1]
		sessionM=a[2]
		sessionS=a[3]
	}

	NF == 9 && match($7, esito) {

		       ip=$3
		   policy=$4
		   filter=$5
		operation=$6
		   result=$7
		   reason=$8

		split($2, a, " ")
		data=a[1]
		time=a[2]

		split(data, a, "/")
		dd=a[1]
		mm=a[2]
		yy=a[3]
		data=yy "/" mm "/" dd

		file_req=filter "_" policy "_" sessiondd sessionmm sessionyy "_" sessionH sessionM sessionS ".req"

		records[++counter] = ip sortfs data sortfs time sortfs policy sortfs filter sortfs operation sortfs \
									result sortfs reason sortfs file_req
	}

	END {
		asort(records)

		for (i = 1; i <= counter; i++) {

			split(records[i], a, sortfs)
			       ip=a[1]
			     data=a[2]
			     time=a[3]
			   policy=a[4]
			   filter=a[5]
			operation=a[6]
			   result=a[7]
			   reason=a[8]
			 file_req=a[9]

			split(data, a, "/")
			yy=a[1]
			mm=a[2]
			dd=a[3]
			data=dd "/" mm "/" yy

			split(time, a, ":")
			H=a[1]
			M=a[2]
			S=a[3]

			file_res=filter "_" policy "_" ip "_" dd mm yy "_" H M S ".res"

			print ip ofs data ofs time ofs policy ofs filter ofs operation ofs result ofs reason ofs file_req ofs file_res
		}
	}

' ofs='\t' esito="$value"
