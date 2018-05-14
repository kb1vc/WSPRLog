#!/bin/bash
# some are prolific two-receiver stations
fname=$1
basename=$2

# split the file into logs
WSPRLogSplitter --igz on ${fname} ${basename}
for bf in ${basename}_*.csv
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
    # now generate the splits with problematic calls removed
    WSPRLogLineFilter ${rfn}_img.csv ${rfn}_img
    # 
    for bif in ${rfn}_img_D.csv
    do
	echo "    ${bif}"
	hfbn=`basename ${bif} .csv`
	# calculate the relative risk for exception reports by solar hour
	WSPRLogSolTimeOR ${bf}_clean.csv ${bif} ${hfbn}_sol_time_or.dat
	WSPRLogHisto ${bif} ${hfbn}_FD.hist --field FREQ_DIFF
	WSPRLogXY ${bif} ${hfbn}_DIST_FD.xydat --x_field FREQ_DIFF --y_field DIST
    done
done


