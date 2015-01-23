awk '
  /^[^#]/ {
    print "+" length($1) "," length($2) ":" $1 "->" $2
  }
  END {
    print ""
  }
' | $HOME/cdbmake "$@"
