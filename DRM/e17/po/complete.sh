#!/bin/sh

while [ 1 ]; do
  FILE="$1"
  shift
  if test -z "$FILE"; then
    break
  else
    TOTAL=`grep '^msgid ' $FILE | wc -l`
    MISSING=`grep '^msgstr ""' $FILE | wc -l`
    DONE=`expr $TOTAL - $MISSING`
    COMPLETE=`expr $DONE \\* 100 / $TOTAL`
    echo "$FILE: $DONE/$TOTAL = $COMPLETE%"
  fi
done
