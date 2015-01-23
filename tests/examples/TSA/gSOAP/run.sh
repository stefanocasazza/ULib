#!/bin/sh

# -----------------------------------------------------------------------------------------------------------------------
# Usage: wsdl2h [-c] [-e] [-f] [-g] [-h] [-I path] [-l] [-m] [-n name] [-N name] [-p] [-r proxyhost:port]
#					 [-s] [-t typemapfile.dat] [-u] [-v] [-w] [-x] [-y] [-o outfile.h] infile.wsdl infile.xsd http://www... ...
# -----------------------------------------------------------------------------------------------------------------------
# -c      generate C source code
# -e      don't qualify enum names
# -f      generate flat C++ class hierarchy
# -g      generate global top-level element declarations
# -h      display help info
# -Ipath  use path to find files
# -l      include license information in output
# -m      use xsd.h module to import primitive types
# -nname  use name as the base namespace prefix instead of 'ns'
# -Nname  use name as the base namespace prefix for service namespaces
# -ofile  output to file
# -p      create polymorphic types with C++ inheritance with base xsd__anyType
# -rhost:port connect via proxy host and port
# -s      don't generate STL code (no std::string and no std::vector)
# -tfile  use type map file instead of the default file typemap.dat
# -u      don't generate unions
# -v      verbose output
# -w      always wrap response parameters in a response struct (<=1.1.4 behavior)
# -x      don't generate _XML any/anyAttribute extensibility elements
# -y      generate typedef synonyms for structs and enums
# infile.wsdl infile.xsd http://www... list of input sources (if none use stdin)
# -----------------------------------------------------------------------------------------------------------------------
# wsdl2h -c -nns -Nns -o tsa.h tsa.wsdl
# -----------------------------------------------------------------------------------------------------------------------
# Usage: soapcpp2 [-1|-2] [-C|-S] [-L] [-c] [-d path] [-e] [-h] [-i] [-I path:path:...] [-m] [-n] [-p name]
#						[-t] [-v] [-w] [-x] [infile]
# -----------------------------------------------------------------------------------------------------------------------
# -1      generate SOAP 1.1 bindings
# -2      generate SOAP 1.2 bindings
# -e		 generate SOAP RPC encoding style bindings
# -C		 generate client-side code only
# -S		 generate server-side code only
# -L		 don't generate soapClientLib/soapServerLib
# -c      generate C source code
# -i      generate service proxies and objects inherited from soap struct
# -dpath  use path to save files
# -Ipath  use path(s) for #import
# -m      generate modules (experimental)
# -n      use service name to rename service functions and namespace table
# -pname  save files with new prefix name instead of 'soap'
# -t      generate code for fully xsi:type typed SOAP/XML messaging
# -w		 don't generate WSDL and schema files
# -x		 don't generate sample XML message files
# -h		 display help info
# -v		 display version info
# infile	header file to parse (or stdin)
# -----------------------------------------------------------------------------------------------------------------------
# soapcpp2 -2 -c -C -t -w -x tsa.h
# -----------------------------------------------------------------------------------------------------------------------

# exit 0

 cd TSA/gSOAP

if [ ! -x ./tsa_client ]
then
	gcc -DWITH_OPENSSL -o tsa_client client.c soapC.c soapClient.c stdsoap2.c -lssl -lcrypto

	strip ./tsa_client
fi

#URL="10.30.1.131:4433"
 URL="https://10.30.1.131:4433/"

if [ ! -f request.tsq ]
then
	echo "pippo pluto paperino" | ../bin/openssl ts -query -config ../openssl.cnf -out request.tsq -cert >/dev/null 2>&1
#											../bin/openssl ts -query								-in  request.tsq -text
   cat ../request/richiesta.hdr request.tsq > ../request/richiesta
fi

#if [ ! -f request.b64 ]
#then
#openssl base64 -e -in request.tsq -out request.b64 >/dev/null 2>&1
#fi

./tsa_client $URL "1" "" "" < request.tsq > response.tsr

../bin/openssl ts -verify -queryfile request.tsq -in response.tsr -CApath ../CA/CApath -token_in
