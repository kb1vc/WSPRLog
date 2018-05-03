#!/bin/bash 

export fname=$1
tref=$2
echo "Filename ${fname}"
export xlab="Time of Day (local to ${tref})"

gnuplot <<\EOF
set title "Reports by Time of Day and Frequency Difference"
set palette gray negative
set xrange [0:24]
set yrange [-100:100]
xl=system("echo $xlab")
set xlabel xl
set ylabel "Image Frequency (Hz)"
fn=system("echo $fname")
print "FOO!"
print fn
print "BAR!"
plot fn using 1:2:3 with image
pause 1000
EOF
