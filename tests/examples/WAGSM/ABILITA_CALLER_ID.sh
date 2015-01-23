#/bin/bash

# ABILITA_CALLER_ID.sh

# set -x

# +CRING: VOICE
# +CLIP: "+39055340773",145,,,,0

 CLIP=`grep 'CLIP:' ./minicom.log | cut -d '"' -f 2`
#CLIP="+393397363258"

echo -n > ./minicom.log

CALLER_ID=${CLIP:3}

# ENVIRONMENT

#export DEBUG=1
export HOME=WAGSM
export LDAP_HOST=192.168.220.11
export LDAP_PASSWORD=programmer
export MAIL_FROM="gsmbox@auth.test.t-unwired.com"
export MAIL_TO="card-activation@auth.test.t-unwired.com"

OUTPUT=`WAGSM/WAGSM_command/card_activation.sh $CALLER_ID`

SMSData=`echo "$OUTPUT" | perl -e "use URI::Escape; print uri_escape(join('',<>));"`

# Sotto i dati per il servizio di test "Comune di Firenze", con 100 sms precaricati.
# -----------------------------------------------------------------------------------------
# la stringa da usare per la chiamata get al gateway sms e'

REQUEST="http://212.131.251.60/bulk/send.asp?Account=comunefirenze&Password=pontevecchio&Sender=COM.FIRENZE&Recipients=1&PhoneNumbers=+39$CALLER_ID&SMSData=$SMSData"

# Specifico i dettagli dei campi inclusi nell'URL sopra riportato:

# Account=comunefirenze					(obbligatorio minuscolo)
# Password=pontevecchio					(obbligatorio minuscolo)
# Sender=COM.FIRENZE						(di esempio, max 11 caratteri)
# Recipients=1                      (per l'invio immediato obbligatorio a 1)
# PhoneNumbers=+393483430356			(+39 obbligatorio)
# SMSData=servizio sms wifi attivo	(di esempio, testo messaggio max 160 caratteri)

# Una nota sulla lunghezza, visto che potrebbe essere gestito in piu` lingue:

# Il messaggio ha una dimensione fissa di 140byte. Questo si traduce in pratica 
# nella possibilita` di usare 160 caratteri di testo (a 7 bit) se si utilizza 
# l'alfabeto latino. In lingue che usano altri caratteri, per esempio russo, 
# cinese e giappone, il messaggio e` limitato a soli 70 caratteri (ognuno di 2 
# byte, usando il sistema Unicode)....quindi il consiglio e`: state sotto i 70 caratteri!!!
# -----------------------------------------------------------------------------------------

# we send request to...
  echo "$REQUEST" >> ./request
# ( echo "$REQUEST" | perl -e "use LWP::Simple; get(join('',<>));" ) >/dev/null 2>/dev/null
