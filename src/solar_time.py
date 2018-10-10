#!/bin/env python
# from https://stackoverflow.com/questions/13314626/local-solar-time-function-from-utc-and-longitude
import sys
from datetime import datetime, time, timedelta
from math import pi, cos, sin

def solar_time(dt, longit):
    return ha

def main():
    if len(sys.argv) != 4:
        print 'Usage: hour_angle.py [YYYY/MM/DD] [HH:MM:SS] [longitude]'
        sys.exit()
    else:
        dt = datetime.strptime(sys.argv[1] + ' ' + sys.argv[2], '%Y/%m/%d %H:%M:%S')
        longit = float(sys.argv[3])

    gamma = 2 * pi / 365 * (dt.timetuple().tm_yday - 1 + float(dt.hour - 12) / 24)
    eqtime = 229.18 * (0.000075 + 0.001868 * cos(gamma) - 0.032077 * sin(gamma) \
             - 0.014615 * cos(2 * gamma) - 0.040849 * sin(2 * gamma))
    decl = 0.006918 - 0.399912 * cos(gamma) + 0.070257 * sin(gamma) \
           - 0.006758 * cos(2 * gamma) + 0.000907 * sin(2 * gamma) \
           - 0.002697 * cos(3 * gamma) + 0.00148 * sin(3 * gamma)
    time_offset = eqtime + 4 * longit
    tst = dt.hour * 60 + dt.minute + dt.second / 60 + time_offset
    solar_time = datetime.combine(dt.date(), time(0)) + timedelta(minutes=tst)
    print solar_time

if __name__ == '__main__':
    main()
