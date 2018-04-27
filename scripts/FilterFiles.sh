#!/bin/bash
# some are prolific two-receiver stations
fname=$1
IFS=","
for tpl in 10.0,10.4,30m 14.0,14.5,20m 7.0,7.5,40m
do
    set -- $tpl
    echo "lo $1  hi $2 band $3"
    WSPRLogBandFilter --flo $1 --fhi $2 $fname wspr_$3_raw.csv multi_remove_$3.rpt    
    grep -v -f multi_remove_$3.rpt < wspr_$3_raw.csv > wspr_$3.csv 
done
    


