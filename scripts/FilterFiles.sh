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
    WSPRLogBandFilter --flo 0.0 --fhi 100e9 ${bf} ${rfn}_pre.csv tmp_remove.lis
    echo "VR2BG" >> tmp_remove.lis
    grep -v -f tmp_remove.lis ${rfn}_pre.csv > ${rfn}_img.csv
    # now generate the splits
    WSPRLogLineFilter ${rfn}_img.csv ${rfn}_img
    for bif in ${rfn}_img_*.csv
    do
	echo "    ${bif}"
	hfbn=`basename ${bif} .csv`
	WSPRLogHisto ${bif} ${hfbn}_FD.hist --field FREQ_DIFF
	WSPRLogXY ${bif} ${hfbn}_DIST_FD.xydat --x_field FREQ_DIFF --y_field DIST
    done
done


