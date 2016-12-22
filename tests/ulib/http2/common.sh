# Copyright (c) 2016 Dridi Boukelmoune
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.

set -e
set -u

TEST_NAM="$(basename "$0")"
TEST_DIR="$(dirname "$0")"
TEST_TMP="$(mktemp -d cashpack.XXXXXXXX)"

HDECODE=hdecode

test -x ./ngdecode && HDECODE+=' ngdecode'

trap "rm -fr $TEST_TMP" EXIT

MEMCHECK_CMD="valgrind			\
	--tool=memcheck			\
	--leak-check=full		\
	--show-leak-kinds=all		\
	--errors-for-leak-kinds=all	\
	--track-fds=yes			\
	--error-exitcode=99		\
	--log-file=memcheck-${TEST_NAM}-%p.log"

set |
grep '^MEMCHECK=' >/dev/null ||
MEMCHECK=no

set |
grep '^NGHTTP2=' >/dev/null ||
NGHTTP2=no

[ "$MEMCHECK" = yes ] && rm -f memcheck-${TEST_NAM}-*.log*

memcheck() {
	if [ "$MEMCHECK" = yes ]
	then
		local rc=0
		$MEMCHECK_CMD "$@" || rc=$?
		[ $rc -eq 99 ] && echo >&2 memcheck: "$@"
		return $rc
	else
		"$@"
	fi
}

cmd_check() {
	missing=

	for cmd
	do
		command -v "$cmd" >/dev/null 2>&1 ||
		missing="$missing $cmd"
	done

	if [ -n "$missing" ]
	then
		echo "program not found:$missing" >&2
		return 1
	fi
}

rm_comments() {
	sed -e '/^#/d'
}

rm_blanks() {
	sed -e '/^$/d'
}

mk_hex() {
	"$TEST_DIR/hex_decode" |
	"$TEST_DIR/hex_encode" >"$TEST_TMP/hex"
	"$TEST_DIR/hex_decode" <"$TEST_TMP/hex" >"$TEST_TMP/bin"
}

mk_bin() {
	cut -d '|' -f 1 |
	while read bin
	do
		# XXX: is this portable?
		dec="$(echo "obase=10;ibase=2;$bin" | bc)"
		printf %02x "$dec"
	done |
	mk_hex
}

mk_msg() {
	rm_comments | rm_blanks >"$TEST_TMP/msg"
	echo >> "$TEST_TMP/msg"
}

mk_tbl() {
	msg="Dynamic Table (after decoding):"
	rm_comments >"$TEST_TMP/tbl_tmp"

	if [ -s "$TEST_TMP/tbl_tmp" ]
	then
		printf "%s\n\n" "$msg" |
		cat - "$TEST_TMP/tbl_tmp" >"$TEST_TMP/tbl"
	else
		printf >"$TEST_TMP/tbl" "%s empty.\n" "$msg"
	fi

	rm  "$TEST_TMP/tbl_tmp"
}

mk_enc() {
	rm_comments | rm_blanks >"$TEST_TMP/enc"
}

mk_chars() {
	fmt="$(printf %s "$2" | tr ' ' '\t')"
	printf "$fmt" ' ' |
	tr '\t ' " $1"
}

hpack_decode() {
	printf "hpack_decode: %s\n" "$*" >&2
	memcheck "$@" "$TEST_TMP/bin" >"$TEST_TMP/dec_out"
}

hpack_encode() {
	printf "hpack_encode: %s\n" "$*" >&2
	memcheck "$@" <"$TEST_TMP/enc" \
		1>"$TEST_TMP/enc_bin" \
		3>"$TEST_TMP/enc_tbl"
}

skip_diff() {
	for opt
	do
		[ "$opt" = --expect-error ] && return
	done
	return 1
}

tst_decode() {
	[ "$NGHTTP2" = yes ] || HDECODE=hdecode
	for dec in $HDECODE
	do
		hpack_decode ./$dec "$@"

		skip_diff "$@" && continue

		printf "Decoded header list:\n\n" |
		cat - "$TEST_TMP/msg" "$TEST_TMP/tbl" >"$TEST_TMP/out"

		diff -u "$TEST_TMP/out" "$TEST_TMP/dec_out"
	done
}

tst_encode() {
	hpack_encode ./hencode "$@"

	skip_diff "$@" && return

	"$TEST_DIR/hex_encode" <"$TEST_TMP/enc_bin" >"$TEST_TMP/enc_hex"

	diff -u "$TEST_TMP/hex" "$TEST_TMP/enc_hex"
	diff -u "$TEST_TMP/tbl" "$TEST_TMP/enc_tbl"
}

repeat() {
	i=1
	j="$1"
	shift
	while [ "$i" -lt "$j" ]
	do
		"$@" $i
		i=$((i + 1))
	done
}

_() {
	if expr "$1" : '^-' >/dev/null
	then
		echo "------$*"
	else
		echo "TEST: $*"
	fi
}
