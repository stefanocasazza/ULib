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
# wsdl2h -c -nns -Nns -o csp.h csp.wsdl
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
# soapcpp2 -2 -c -C -t -w -x csp.h
# -----------------------------------------------------------------------------------------------------------------------

# exit 0

cd CSP/gSOAP

if [ ! -x ./csp_client ]
then
	gcc -DWITH_OPENSSL -o csp_client client.c soapC.c soapClient.c stdsoap2.c -lssl -lcrypto

	strip ./csp_client
fi

CNF=`cat openssl.cnf`
P10=`cat newreq.pem`
#SPKAC=`cat SPKAC.pem`
#URL="10.30.1.131:4433"
 URL="https://10.30.1.131:4433/"

 ./csp_client $URL 1 CA   300
 ./csp_client $URL 1 CA_1 400 "$CNF"
 ./csp_client $URL 1 CA   300
 ./csp_client $URL 2
 ./csp_client $URL 3 CA policy_anything "$P10"
#./csp_client $URL 4 CA policy_anything "$SPKAC"
 ./csp_client $URL 5 CA 1
 ./csp_client $URL 6 CA "01"
 ./csp_client $URL 3 CA_1 policy_anything "$P10"
 ./csp_client $URL 7 CA
