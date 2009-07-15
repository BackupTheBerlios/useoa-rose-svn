#! /bin/bash

MAXMEMUSG=0

while [[ 1 ]] ; do
    PID=`ps -A | grep "OATest" | gawk '{print $1}' /dev/stdin`
    if [[ $PID = "" ]]; then
        break
    fi
    MEMUSG=`ps -p $PID -o "vsize" | tail -1`

    if (( $MEMUSG > $MAXMEMUSG )); then
       MAXMEMUSG=$MEMUSG
    fi

    sleep 0.2
done

echo "Peak memory usage = $MAXMEMUSG"

