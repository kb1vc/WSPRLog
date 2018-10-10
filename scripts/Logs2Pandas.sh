#!/bin/bash -v
# some are prolific two-receiver stations
fname=$1
basename=$2


# split the file into logs
#  Remove duplicate heavy stations
# add solar time columns
# add "is duplicate" column
WSPRLogSplitter --igz on ${fname} ${basename}
# for right now, just do 30m
for bf in ${basename}_30m.csv
do
    rfn=`basename ${bf} .csv`
    echo "processing ${bf}"
    WSPRLogBandFilter --flo 0.0 --fhi 100e9 ${bf} ${rfn}_img_tmp.csv
    # figure out which calls should be removed.
    #    make a temporary band split file
    WSPRLogLineFilter ${rfn}_img_tmp.csv ${rfn}_img_tmp
    #    gather the calls that are likely to be multi-reporters
    CallRR ${bf} ${rfn}_img_tmp_D.csv ${rfn}_callrr.rpt
    grep '^H' rx_${rfn}_callrr.rpt | awk '{ print $2; }' | sort > ${rfn}_exclude_calls.lis
    # now remove the possible problem RX stations
    grep -v -f ${rfn}_exclude_calls.lis ${rfn}_img_tmp.csv > ${rfn}_img.csv
    grep -v -f ${rfn}_exclude_calls.lis ${bf} > ${rfn}_clean.csv
    # remove the junk files
    rm ${rfn}_img_tmp.csv [rt]x_${rfn}_callrr.rpt ${rfn}_img_tmp_*.csv 

    # now we have a CLEAN log with the clearly problematic calls eliminated. 
    # build a log with solar times, and image reports. 
    WSPRLog2Pandas ${rfn}_clean.csv ${rfn}_pnd.csv_tmp
    sed -e 's/|/,/' < ${rfn}_pnd.csv_tmp > ${rfn}_pnd.csv
    rm ${rfn}_pnd.csv_tmp
done


