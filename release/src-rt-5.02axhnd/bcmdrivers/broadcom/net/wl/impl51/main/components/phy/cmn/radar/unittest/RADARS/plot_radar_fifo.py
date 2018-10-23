#!/usr/bin/env python
#
# $ Copyright Broadcom Corporation $
#
#
# <<Broadcom-WL-IPTag/Proprietary:.*>>
#
# $Id$

import numpy as np
import sys,os
import pylab as plt

#
# Get BW and version from filenames on cmd line
#

fns = sys.argv[1:]

plt.figure()

for fn in fns:
    fnsplit = fn.split('_')

    bw = int(fnsplit[-2][2:])
    bwk = (bw / 20) >> 1

    version = fnsplit[0][5:]
    if version == "" : 
        version = 1
    else: 
        version = int(version)
        bwk = 0 # 
    if version == 1:
        fiforaw = np.fromstring(open(fn,'rb').read(),dtype=np.uint16 )
        fifo = np.vstack( (fiforaw[0::4],fiforaw[1::4],fiforaw[2::4],fiforaw[3::4] )).transpose()
        # unpack fifo to T0 T2 and FMdiff
        # (bits High to Low)
        # First Word:	Time0[31:16]
        # Second Word:	Time2[3:0] | Time0[15:4]
        # Third Word:	FMDiff[7:0] | Time2[11:4]
        # Fourth Word:	0x00  00 FMDiff[8] | Time2[12] | Time0[3:0]
        desc = np.dtype([('T0', 'uint32'), ('T2', 'uint16'),  ('FMdiff', 'int16')])
        uf = np.zeros(shape=(fifo.shape[0],), dtype=desc )

    elif version == 2:
        fiforaw = np.fromstring(open(fn,'rb').read(),dtype=np.uint16 )
        fifo = np.vstack( (fiforaw[0::6],fiforaw[1::6],fiforaw[2::6],fiforaw[3::6],fiforaw[4::6],fiforaw[5::6] )).transpose()
        # unpack fifo to T0 T2 and FMdiff
        # (bits High to Low)
        # First Word:	Time0[31:16]
        # Second Word:	Time2[3:0] | Time0[15:4]
        # Third Word:	FMDiff[7:0] | Time2[11:4]
        # Fourth Word:	0x00  00 FMDiff[8] | Time2[12] | Time0[3:0]
        desc = np.dtype([('T0', 'uint32'), ('T2', 'uint16'),  ('FMdiff', 'int16'),  ('FC', 'int16'),  ('CHIRP', 'int16'),  ('NOTRADAR', 'int16')  ])
        uf = np.zeros(shape=(fifo.shape[0],), dtype=desc )

    else:
        print "Unknow version found from %s" %fn
        exit()
#
#f0       <= std_logic_vector(timer(31 downto 16)) when (phyregs2radarDet.mode_4360b0 = '0') else std_logic_vector(timer_shift(31 downto 16));
#  f1       <= std_logic_vector(timer2(3 downto 0)) & t0save(15 downto 4)  when (phyregs2radarDet.mode_4360b0 = '0')
#              else std_logic_vector(timer2_shift(3 downto 0)) & t0save_shift(15 downto 4);
#
#  f2       <= f2_mux_val & t2save(11 downto 4)  when (phyregs2radarDet.mode_4360b0 = '0')
#             else  std_logic_vector(fmdemod_diff_sat(7 downto 0)) & t2save_shift(11 downto 4);
#
#  f2_mux_val    <= (std_logic_vector(timer3(3 downto 0)) & t1save) when (phyregs2radarDet.fmdemodEnable = '0')
#                   else std_logic_vector(fmdemod_diff_sat(7 downto 0)) when phyregs2radarDet.fmDetMod = '0'
#                   else fmDetAccSat9(7 downto 0);
#
#  f3       <= X"00" & "00" & f3_mux_val & t2save(12) & std_logic_vector(t0save(3 downto 0)) when (phyregs2radarDet.mode_4360b0 = '0')
#              else "0"& fmDetAccSat9(8 downto 0) & fmdemod_diff_sat(8) & t2save_shift(12) & t0save_shift(3 downto 0);
#
#  f3_mux_val <= fmdemod_diff_sat(8) when phyregs2radarDet.fmDetMod = '0' else fmDetAccSat9(8);
#


# T0 uint32 Ensure we dont loose MSBs while converting 16bits to 32bits!
    uf['T0'] = ( (fifo[:,0] * (1 << 16)) + ((fifo[:,1] & 0x0FFF) << 4) + (fifo[:,3] & 0x000F) ) >> bwk
# T2 uint16
    uf['T2'] = ( ((fifo[:,1] & 0xF000) >> 12) | ((fifo[:,2] & 0x00FF) << 4 ) | ((fifo[:,3] & 0x0010) << 8) ) >> bwk
# FMdiff int9?
    uf['FMdiff'] = ((fifo[:,2] & 0xFF00) >> 8) | ((fifo[:,3] & 0x20) << 3 ) ## ??
    TOA = uf['T0']/20.
    PW  = uf['T2']/20.
    FM = uf['FMdiff']  # plot RAW FM

    if version == 2:
        uf['FC']  = fifo[:,4] & 0x1FF 
        uf['CHIRP']   = (fifo[:,3] & 0x7fc0) >> 6
        uf['NOTRADAR'] = (fifo[:,5] & 0xFFF8) >> 3
        # print uf.dtype
        # for samp in uf:
        #    print samp
        FC = uf['FC']  # 
        CHIRP = uf['CHIRP']  # 
        NOTRADAR = uf['NOTRADAR']  # 

#
#  Reconstruct actual pulses
#  Let every pulse have 4  data points:
#  y =  0   1   1      0
#  x =  T0  T0  T0+PW  T0+PW
    y = np.zeros(shape=(TOA.shape[0],4))
    x = np.zeros(shape=(TOA.shape[0],4))
    y[:,0] = y[:,3] = 0.
    y[:,1] = y[:,2] = 1.
    x[:,0] = x[:,1] = TOA
    x[:,2] = x[:,3] = TOA +PW

    ax = plt.subplot('711')
    plt.plot(np.ravel(x), np.ravel(y), marker='o', markersize=2)
    plt.ylabel('Pulse')
    plt.xlabel('Time(usec)')

    plt.subplot('712', sharex=ax)
    plt.plot(TOA, PW, marker='o', markersize=2)
    plt.ylabel('Pulse Width(usec)')
    plt.xlabel('Time(usec)')

    ax2 = plt.subplot('713', sharex=ax)
    plt.plot(TOA[1:], np.diff(TOA), marker='o', markersize=2)
    #ax2.set_ylim(ymin=0, ymax=4000)
    plt.ylabel('Pulse Intervals(usec)')
    plt.xlabel('Time(usec)')

    plt.subplot('714', sharex=ax)
    plt.plot(TOA, FM,  marker='o', markersize=2)
    plt.ylabel('FMdiff')
    plt.xlabel('Pulse arrival(usec)')

    if version == 2:

        plt.subplot('715', sharex=ax)
        plt.plot(TOA, FC,  marker='o', markersize=2)
        plt.ylabel('FC')
        plt.xlabel('Pulse arrival(usec)')

        plt.subplot('716', sharex=ax)
        plt.plot(TOA, CHIRP,  marker='o', markersize=2)
        plt.ylabel('chirp')
        plt.xlabel('Pulse arrival(usec)')
        
        plt.subplot('717', sharex=ax)
        plt.plot(TOA, NOTRADAR,  marker='o', markersize=2)
        plt.ylabel('NotRadar??')
        plt.xlabel('Pulse arrival(usec)')

plt.show()
