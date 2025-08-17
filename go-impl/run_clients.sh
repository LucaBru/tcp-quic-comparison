#!/bin/bash

CMD1="$1"
CMD2="$2"

[ -n "$CMD1" ] && (eval $CMD1) &
pid1=$!

[ -n "$CMD2" ] && (eval $CMD2) &
pid2=$!

[ -n "$pid1" ] && wait $pid1
[ -n "$pid2" ] && wait $pid2
