#/bin/bash

# card_activation.sh
set -x

(
CALLER_ID=`echo -n $1 | perl -e "use URI::Escape; print uri_escape(join('',<>));"`

exec /home/unirel/userver/bin/send_req_to_portal.sh gen_activation "gen_activation?caller_id=$CALLER_ID"
)>& /tmp/attivation.err
