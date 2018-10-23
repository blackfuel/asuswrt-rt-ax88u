#!/usr/bin/env python
#
# $ Copyright Broadcom Corporation $
#
#
# <<Broadcom-WL-IPTag/Proprietary:.*>>
#
# $Id$

import os, sys

import numpy as np
import pylab as plt

from optparse import OptionParser,OptionValueError

from struct import pack,unpack

import pdb
#pdb.set_trace()

def rms(rx1):
    return  np.sqrt(np.sum(np.abs(rx1)**2)/len(rx1))


def array2wv(filename, B, clk = 40, bw = None, comment="None", packet_samples=0):
    """ convert and array to a smu wv file

    we assume the array is scaled to mac +-1

    """

    A      = np.asarray(B,dtype='c8').copy()

    corr = 20*np.log10(rms(A[:packet_samples])/rms(A))

    comment += " dB=%.1f"%corr

    peak_dB = 20*np.log10(32768.0/np.abs(A).max())
    rms_dB  = 20*np.log10(32768.0/ rms(A[:packet_samples]))

    A *= (1<<15)

    A.real = A.real.clip(-32768,32767)
    A.imag = A.imag.clip(-32768,32767)

    fd = file(filename,"wb")
    fd.write("{TYPE: SMU-WV,0}\n")
    fd.write("{COMMENT: %s}\n"%(comment))
    fd.write("{CLOCK: %.6fe6}\n"%(clk))
    fd.write("{SAMPLES: %d}\n"%(A.size))
    if bw:
        fd.write("{FILTER: %.1fMHz}\n"%bw)

    fd.write("{LEVEL OFFSET: %.1f, %.1f}\n"%(rms_dB, peak_dB))                 # does not seem to have an effect

    fd.write("{WAVEFORM-%d: #"%(len(A)*4+1))
    # now add the data

    for a in A:
        i =np.uint16(a.real)
        q =np.uint16(a.imag)

        fd.write(pack('H',i))
        fd.write(pack('H',q))

    fd.write("}\n")

    fd.close()
    print "created WV file '%s'"%filename
    return A

def ScaleIBUF(IREF, ICPY, dBr, debug = False):
    # Scale rx_ibuf aka adc samples by dBr
    orig_samples = np.memmap(IREF, dtype='complex128', mode='r')
    new_samples =  np.memmap(ICPY,   dtype='complex128', mode='write', shape = orig_samples.shape )
    # mode = ['r', 'c', 'r+', 'w+', 'readwrite', 'write', 'readonly', 'copyonwrite' ]
    new_samples[:] = orig_samples[:]*pow( 10.0, dBr/20.0 )
    new_samples.flush()
    # Recover memory...
    del orig_samples
    del new_samples
    if debug:
       print "Orig: %f dBm, Scaled: %f  dBm, dBr used: %s"  %(ExtractPktPwr(IREF,2),ExtractPktPwr(ICPY,2),dBr)
    return



def GenFMburst_IBUF(IBUF, SR, dF, PW):
    N = int(SR*PW)
    sig = np.memmap(IBUF, dtype='complex64', shape= (N,), mode='w+')
    # mode = ['r', 'c', 'r+', 'w+', 'readwrite', 'write', 'readonly', 'copyonwrite' ]
    t = np.arange(-N/2, N/2)/SR
    sig[:] = np.exp( +1j*np.pi*dF/PW*(t**2) )
    sig.flush()
    # Recover memory...
    #del sig
    return sig

def GenPW_IBUF(IBUF, SR, PW):
    N = int(SR*PW)
    sig = np.memmap(IBUF, dtype='complex64', shape= (N,), mode='w+')
    # mode = ['r', 'c', 'r+', 'w+', 'readwrite', 'write', 'readonly', 'copyonwrite' ]
    sig[:] = 1. +1j*0.
    sig.flush()
    # Recover memory...
    #del sig
    return sig

def GenIFS_IBUF(IBUF, SR, IFS):
    N = int(IFS*SR)
    sig = np.memmap(IBUF, dtype='complex64', shape= (N,), mode='w+')
    # mode = ['r', 'c', 'r+', 'w+', 'readwrite', 'write', 'readonly', 'copyonwrite' ]
    sig[:] = 0. +1j*0.
    sig.flush()
    # Recover memory...
    #del sig
    return sig

def extract_active_packet_rms(signal, thres_DC_dB, debug = False):
   # Extract active packet from "packet" to calc
    # packet 'power'
    # Calc a signal threshold
    pkt_len = len(signal)
    pwr = signal*np.conjugate(signal)
    dc = np.sqrt(np.mean(pwr))
    thres = dc*pow(10,thres_DC_dB/20)
    # find first sample that exceeds thres
    for i in range(pkt_len):
        pkt_start = i
        if abs(signal[i])>= thres: break
    # find last reversing from end that exceeds thres
    for i in range(pkt_len-1,0,-1):
        pkt_end = i
        if abs(signal[i])>= thres: break
    # Calc active packet power
    pkt_sigV = signal[pkt_start:pkt_end]*np.conjugate(signal[pkt_start:pkt_end])
    pkt_rms = np.sqrt(np.mean(pkt_sigV)).real
    #sig_mask = [0]*start + [pkt_rms]*(end-start) + [0]*(pkt_len-end)
    return pkt_rms, pkt_start, pkt_end, pkt_len


def ExtractPktPwr(IBUF, num_rx_ant, debug = False):
    # Find antenna with largest power
    try:
        fpr = np.memmap(IBUF, dtype='complex128', mode='r')
    except:
        print "FAILED to estimate packet power"
        return -999.0
    maxPWRdBm = -999.0  # dBm
    for ant in range(num_rx_ant):
         rmsV, start, end, pkt_len = extract_active_packet_rms(fpr[ant::num_rx_ant], -20.0 )
         pwr_dBm = 10.0*np.log10(rmsV*rmsV/50.0) + 30.0
         if pwr_dBm > maxPWRdBm: maxPWRdBm = pwr_dBm
    del fpr
    return maxPWRdBm  ##


if __name__ == "__main__":

    description = "By default ReRun reruns the supershrink-crs40-rx executable\
    on each directory found under -S SEARCH_DIR that contains both params and an rx_ibuf file\
    The binary output(stderr and some stdout) is saved to ReRun.log.\
    The following options modify this behaviour"
    #
    parser = OptionParser(usage = "usage: %prog [options] arg1 arg2", description=description)


    parser.add_option("-D","--debug-script",
                      dest="debug",default = False,
                      help = "Be verbose with debugging output",
                      action="store_true")

    parser.add_option("--force-tones",
                      dest="CW_TONES",default = "",
                      help = "Substitute a CW tone for rx_ibuf",
                      action="store")

    parser.add_option("--lead-us",
                      dest="LEAD_US",default = 0.0,
                      help = "leading silence for tones",
                      action="store")

    (options, args) = parser.parse_args()

    for dFM in [0.1e6, 0.2e6, 0.5e6, 1.0e6, 2.0e6, 5.0e6, 10.0e6, 20.0e6, 40e6 ]:
        for PW in [1e-6, 2e-6, 5e-6, 10e-6, 15e-6, 20e-6, 50e-6, 75e-6, 100e-6, 200e-6]:
            fname = "radar_pulse_fm_chirp_%1.1fMHz_%dus" %(dFM/1e6, PW*1e6)
            df = GenFMburst_IBUF( fname+".dat", 80e6, dFM, PW )
            wv = array2wv(fname+".wv", df, clk = 80, bw = None, packet_samples=0)

    for PW in [1e-6, 10e-6, 100e-6]:
            fname = "radar_pulse_%dus" %( PW*1e6)
            df = GenPW_IBUF( fname+".dat", 80e6, PW )
            wv = array2wv(fname+".wv", df, clk = 80, bw = None, packet_samples=0)

    for IFS in [1e-6, 10e-6, 100e-6, 1000e-6]:
            fname = "radar_ifs_%dus" %( IFS*1e6)
            df = GenIFS_IBUF( fname+".dat", 80e6, IFS )
            wv = array2wv(fname+".wv", df, clk = 80, bw = None, packet_samples=0)

    #del df
    plt.figure()
    plt.subplot(311)
    plt.plot(df.real, df.imag)
    plt.subplot(312)
    t = np.arange(df.size)/80e6
    plt.plot(t, df.real)
    plt.plot(t, df.imag)
    plt.subplot(313)
    plt.plot(10*np.log10(np.fft.fft(df)**2) )
    plt.show()
