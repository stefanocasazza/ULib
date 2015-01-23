#!/bin/bash

#UTRACE="0 45M 0"
#UOBJDUMP="0 100k 10"
#USIMERR="error.sim"
#EXEC_ON_EXIT="/utility/stack_dump.sh"
 export UTRACE UOBJDUMP USIMERR EXEC_ON_EXIT

LOG=log/benchmark.log
TIMEFORMAT=$'*** Real %3lR\tUser %3lU\tSys %3lS\t%P%% CPU'

#DIR="."
 DIR="../.."
#DIR_CMD="$DIR"
 DIR_CMD="$DIR/../../examples/IR"
#. $DIR/.colors
 . $DIR/../.colors

mkdir -p out/ err/ log/

#export CMD=valgrind
#export LD_LIBRARY_PATH=.

print_cmd() {

	# log

	echo "$1" >> $LOG

	# terminal

	$WARNING
	echo $1
	$NORMAL
}

exec_cmd() {

	if [ -z "$CMD" ]; then
		( time $DIR_CMD/"$1" "$2" "$3" "$4" "$5" >out/$1.out 2>err/$1.err ) 2>tmp || \
			{ $FAILURE; echo "*** ERROR ON $1 ***"; $NORMAL; exit 1; }
	else
		( $CMD $DIR_CMD/"$1" "$2" "$3" "$4" "$5" >out/$1.out 2>err/$1.err ) 2>tmp || \
			{ $FAILURE; echo "*** ERROR ON $1 ***"; $NORMAL; exit 1; }
	fi

	# log

	cat tmp >> $LOG
	echo	  >> $LOG

	# terminal

	cat tmp
	echo

	rm -rf tmp
}

# 1 -> 20 directory, 1 directory = 1 cartella, 1 cartella = 20 files

document_generation() {

	print_cmd "DOCUMENT_GENERATION:"

	rm	   -rf $DIR/IR/doc
	mkdir -p  $DIR/IR/doc

 	exec_cmd gendoc $1 $DIR/IR/doc/
}

indexing_mem() {

#  UTRACE="0 80M 0"

	print_cmd "INDEXING(1):"

	rm	   -rf $DIR/IR/db
	mkdir -p  $DIR/IR/db

	exec_cmd index1

#  unset UTRACE
}

indexing_dsk() {

#  UTRACE="1 80M 0"

	print_cmd "INDEXING(2):"

	rm	   -rf $DIR/IR/db
	mkdir -p  $DIR/IR/db

	exec_cmd index2

#  unset UTRACE
}

updating() {

#  UTRACE="0 10M 0"

	DEL_FILE="D01/D01_studente_1.xml,D01/D01_studente_1_op_1.xml"
	ADD_FILE="$DEL_FILE,D01/D01_studente_1_op_2?.xml"
	SUB_FILE="D01/D01_studente_1_op_2?.xml"

	print_cmd "UPDATING del:"

	if [ -f $DIR/IR/doc/D01/D01_studente_1_op_20.xml ]
	then
		exec_cmd update -d "$ADD_FILE"
	else
		exec_cmd update -d "$DEL_FILE"
	fi

	cp $DIR/inp/D01/D01_studente_1.xml \
		$DIR/inp/D01/D01_studente_1_op_1.xml \
		$DIR/inp/D01/D01_studente_1_op_20.xml $DIR/IR/doc/D01/ >/dev/null 2>/dev/null

	print_cmd "UPDATING add + substitute:"

	exec_cmd update -a "$ADD_FILE" -s "$SUB_FILE"

#  unset UTRACE
}

db_check() {

#	UTRACE="0 10M 0"

	print_cmd "DATABASES CHECK:"

	exec_cmd db_check

#	unset UTRACE
}

querying() {

# 	export UTRACE="0 10M 0"

	print_cmd "QUERY: '$1'"

 	exec_cmd query "$1"

	diff -q out/query.out query.exp >/dev/null 2>/dev/null || \
		{ $FAILURE; echo '*** ERROR ON QUERY ***'; $NORMAL; exit 1; }

# 	unset UTRACE
}

profiling() {

	gprof -b $DIR_CMD/$1 gmon.out >profile.out 2>/dev/null
}

sizing_doc() {

	print_cmd "DOCUMENT size:"

	sync
	du -sh $DIR/IR/doc | tee -a $LOG
	echo						    >> $LOG
	echo
}

sizing_db() {

	print_cmd "DATABASE size:"

	sync
	du -sh $DIR/IR/db/* | tee -a $LOG
	echo							  >> $LOG
	echo
}

print_start() {

	# log

	DATE=`date '+%X %D'`

	echo "($DATE) NUMBER OF DOCUMENTS = $NUM"													>> $LOG
	echo "====================================================================="	>> $LOG
	echo																									>> $LOG

	# terminal

	$NORMAL
	$ECHO "($DATE) NUMBER OF DOCUMENTS = "
	$SUCCESS
	$ECHO $NUM
	$NORMAL
	echo
	echo "====================================================================="
	echo
}

print_end() {

	# log

	echo "=====================================================================" >> $LOG
	echo																								  >> $LOG

	# terminal

	$NORMAL
	echo "====================================================================="
	echo
}

chain() {

	# ----------------------------------------------------------------------------------------------------------
	# index.cfg - configuration data for program of Imformation Retrieval
	# ----------------------------------------------------------------------------------------------------------
	# DB							location for index db (must be terminated by /)
	# DIRECTORY					location of docs to index
	# DIMENSION					approximate number of docs to index
	# MIN_WORD_SIZE			sets the mininum length of words that will be indexed
	# IGNORE_CASE           case sensitive or not
	# SKIP_TAG_XML				skip index of tag xml for files with suffix indicated
	# BAD_WORDS             template words to not index for files with suffix indicated in BAD_WORDS_EXT
	# BAD_WORDS_EXT         extension file for BAD_WORDS
	# FILTER_EXT            preprocessing for files with suffix indicated
	# FILTER_CMD            preprocessing command for files with suffix indicated in FILTER_EXT
	# ----------------------------------------------------------------------------------------------------------

	NUM=`expr $1 \* 400`

	cat << EOF > index.cfg
INDEX_CFG
	{
	DB					$DIR/IR/db/ # must be terminated by /
	DIRECTORY		$DIR/IR/doc
	DIMENSION		$NUM
	MIN_WORD_SIZE	3

	IGNORE_CASE    yes

 	SKIP_TAG_XML   "[ xml ]"

 	BAD_WORDS		??/??/????|??:??:??|workflow
#  BAD_WORDS_EXT  "[ xml ]"

#  FILTER_EXT "[   pdf                                    doc                  html ]"
#  FILTER_CMD "[ \"pdftotext -raw -nopgbrk -q $FILE -\" \"catdoc -aw $FILE\" \"htuml2txt\"
	}
EOF

	print_start

 	document_generation $1

	sizing_doc

#	indexing_mem
 	indexing_dsk

 	sizing_db

  	updating

  	querying	'D01_studente_1_op_20'
 	querying	'D01_studente_1_op_2?'
  	querying	'Ritiro certificato20'
  	querying	'"Ritiro certificato20"'
 	querying	'(D01_studente_1_op_20 AND Ritiro AND certificato20)'
  	querying	'(D01_studente_1_op_20 AND "Ritiro certificato20")'
 	querying	'(D01_studente_1_op_20 AND Ritiro AND certificato20) AND NOT 01_rossi_2'
	querying	'(D01_STUDENTE_1_OP_20 and RITIRO and Certificato20) and NOT 01_Rossi_2'
	querying '(*Studente*20 AND Certificato??) AND NOT ??_Rossi*2'

# 	querying	'<id>D01_studente_1_op_20</id>'
#	querying	'<id>D01_studente_1_op_2?</id>'
# 	querying	'<description>Ritiro certificato</description>'
# 	querying	'"<description>Ritiro certificato</description>"'
#	querying	'(<id>D01_studente_1_op_20</id> AND <description>Ritiro AND certificato</description>)'
# 	querying	'(<id>D01_studente_1_op_20</id> AND "<description>Ritiro certificato</description>")'
#	querying	'(<id>D01_studente_1_op_20</id> AND <description>Ritiro AND certificato</description>) AND NOT 01_rossi_2'

#	profiling query

 	db_check

	print_end
}

 if [ -f $LOG ]
 then
	mv $LOG $LOG.pre
 fi

 rm -rf out/* err/* gmon.out profile.out \
		  trace.*.[0-9]* \
        object.*.[0-9]* \
        stack_dump.*.[0-9]*

#chain    1 #    400
 chain    2 #    800
#chain   20 #   8000
#chain   50 #  20000
#chain  100 #  40000
#chain  150 #  60000
#chain  200 #  80000
#chain  250 # 100000
#chain  300 # 120000
#chain  600 # 240000
#chain  800 # 320000
#chain 1000 # 400000
