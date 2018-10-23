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
import random
import copy
from optparse import OptionParser,OptionValueError

description = "Parses case file and returns case numbers that match re argumenets"

parser = OptionParser(usage = "usage: %prog [options] arg1 arg2", description=description)

parser.add_option("-v", "--version",
                  dest="version", default=1, type=int,
                  help = "PHY version to emulate",
                  action="store")
 
parser.add_option("-s", "--seed",
                  dest = "seed", default = 1, type = int,
                  help = "Seed for random number generator",
                  action="store")

(options, args) = parser.parse_args()



def FCCRoundUp(I): return np.ceil(19e6/360./I)

def midN(A,N,S=1):
    # take N values spaced S about the middle of an array
    mid = A.size/2
    d  = (N*S-S)/2
    #print A.size,mid,N,S, range(mid-d, mid+d+S, S)
    return  A[range(mid-d, mid+d+S, S)]


PHYINFO = {
1: {'FIFO_SIZE':512, 'wordsperpulse':4},
2: {'FIFO_SIZE':512, 'wordsperpulse':6}
}

if options.version==1:  fprefix = "radar" 
else:  fprefix = "radar%d" %options.version

FIFO_SIZE = PHYINFO[options.version]['FIFO_SIZE']
wordsperpulse = PHYINFO[options.version]['wordsperpulse']
maxpulses = np.floor(FIFO_SIZE/wordsperpulse)
# Temp fix to ensure identical common data for both phy revs.
# Generating different num of pulses causes randomness to vary for the variables adjusted.
# Not a valid fix when uncommon data makes calls to random.
maxpulses = 128  


BRI_default = int(3/20.*1e6) # must be  > 75ms and < 3/20sec
# Provide some variation on TOA and PW measurement between antennas/cores
noise = 1  # integer = noise /20 eqivalent 20MHz sample counts noise

rspec = {}

#
# PRI & BRI defined as spacing between rising edge of pulses
#
FCCTable5ATestA_PRI = np.array([518,538,558,578,598,618,638,658,678,698,718,738,758,778,798,818,838,858,878,898,918,938,3066])
FCCTable5ATestA_NP = FCCRoundUp(FCCTable5ATestA_PRI)
# Test A 15 values from Table 5a (23 values)
# Test B 15 values not previously selected???
# Test B not to use any vaules used for Test A ...
# Not explicity stated but it is assumed that each fcc burst has fixed/constant PRI as the number of pulses is determined
# by the PRI

############################################
#
#   FCC ETSI   single PRF/PRI and PW
#   Taken from FCC 13-22
############################################
rspec['fcc-0'] = { 'PW':[[],[1],[]],     'PRI':[[],[1428],[]], 'NP':[[],[18],[]]  }  # CA and Det tests only
rspec['fcc-1'] = { 'PW':[[],[1],[]],     'PRI':[[],FCCTable5ATestA_PRI[11:12],[]],  'NP':[[],FCCTable5ATestA_NP[11:12],[]]  }
rspec['fcc-2'] = { 'PW':[[1],[3],[5]],   'PRI':[[150],[(150+230)/2.],[230]],  'NP':[[23],[26],[29]]  }
rspec['fcc-3'] = { 'PW':[[6],[8],[10]],  'PRI':[[200],[(200+500)/2.],[500]],  'NP':[[16],[17],[18]]  }
rspec['fcc-4'] = { 'PW':[[11],[16],[20]],'PRI':[[200],[(200+500)/2.],[500]],  'NP':[[12],[14],[16]]  }
# Long pulse radar
# 8 to 20 bursts  per 12 sec period  Say 14 bursts in 12 sec evenly spaced
# Spec requires some randomisation of NP, PRI  & PW
rspec['fcc-5'] = { 'PW':[[50],[75],[100]],'PRI':[[1000],[1500],[2000]], 'NP':[[1,2],[3],[2,3]], 'FM':[510], 'BRI':[int(12./20.*1e6)]  }

rspec['fcc-not'] = { 'PW':[0.5],'PRI':[50], 'NP':[20] }


############################################
#
#    ESTI Staggered
#
############################################
#
# Total number of pulse per PRF = NP
# Total number of pulses = NP&NUMPRF
# In a burst of NP pulses the pulses are spaced either
#  a-a-a-a-a-a-a-a
# NUMPRF = 2
#  a-a-a-a-b-b-b-b   stag-aabb sequenced 'seq'
#  a-b-a-b-a-b-a-b   stag-abab alternate 'alt'
# NUMPRF = 3
#  a-a-a-b-b-b-c-c-c stag-aabbcc
#  a-b-c-a-b-c-a-b-c stag-abcabc

# Specs taken from Table D.4 in ETSI EN 301 893 V1.5.1 (2008-12) taken from:
# http://hwnbu-twiki.sj.broadcom.com/twiki/pub/Mwgroup/DFSMainPage/en_301893v010501p.pdf
# NB the Table D.4 uses PRF(pps) not PRI(usec)
# Convert Pulse Repition Frequency to interval (usec)
# NB: arange does not include the stop/end value !
prfstep = 10 # pps
PRIs1 = np.floor(1e6/np.arange(200,1000+prfstep,prfstep))
PRIs2 = np.floor(1e6/np.arange(200,1600+prfstep,prfstep))
PRIs3 = np.floor(1e6/np.arange(2300,4000+prfstep,prfstep))
PRIs4 = np.floor(1e6/np.arange(2000,4000+prfstep,prfstep))
PRIs5 = np.floor(1e6/np.arange(300,400+prfstep,prfstep))   # diff in PRI 20 - 50 pps   set S in midN to 2
PRIs6 = np.floor(1e6/np.arange(400,1200+prfstep,prfstep))  # diff in PRI 80 - 400 pps  set S in midN to 8

# Taken from ETSI EN 301 893 V1.5.1 (2008-12)  Table D.4
#
# Constant PRF types
rspec['etsi-1'] = { 'PW':[[0.8],[1],[5.]],  'PRI':[[PRIs1[0]],midN(PRIs1,1),[PRIs1[-1]]], 'NP':[[],[15],[]] }
rspec['etsi-2'] = { 'PW':[[0.8],[8],[15]],  'PRI':[[PRIs2[0]],midN(PRIs2,1),[PRIs2[-1]]], 'NP':[[],[10],[]] }
rspec['etsi-3'] = { 'PW':[[0.8],[8],[15]],  'PRI':[[PRIs3[0]],midN(PRIs3,1),[PRIs3[-1]]], 'NP':[[],[25],[]] }
rspec['etsi-4'] = { 'PW':[[20],[25],[30]],  'PRI':[[PRIs4[0]],midN(PRIs4,1),[PRIs4[-1]]], 'NP':[[],[20],[]], 'FM':[510] }

rspec['etsi-prf-not'] = { 'PW':[0.5],'PRI':[140], 'NP':[20] } #

# Variable PRF types
rspec['etsi-5-2aabb'] = { 'PW':[[0.8],[1.4],[2]], 'PRI':[PRIs5[[0,1]],midN(PRIs5,2,2),PRIs5[[-1,-2]]], 'NP':[[],[10],[]], 'PRNP':(2,'seq') }
rspec['etsi-5-2abab'] = { 'PW':[[0.8],[1.4],[2]], 'PRI':[PRIs5[[0,1]],midN(PRIs5,2,2),PRIs5[[-1,-2]]], 'NP':[[],[10],[]], 'PRNP':(2,'alt') }

rspec['etsi-5-3aabb'] = { 'PW':[[0.8],[1.4],[2]], 'PRI':[PRIs5[[0,1,2]],midN(PRIs5,3,2),PRIs5[[-1,-2,-3]]], 'NP':[[],[10],[]], 'PRNP':(3,'seq') }
rspec['etsi-5-3abab'] = { 'PW':[[0.8],[1.4],[2]], 'PRI':[PRIs5[[0,1,2]],midN(PRIs5,3,2),PRIs5[[-1,-2,-3]]], 'NP':[[],[10],[]], 'PRNP':(3,'alt') }

rspec['etsi-6-2aabb'] = { 'PW':[[0.8],[1.4],[2]], 'PRI':[PRIs6[[0,1]],midN(PRIs6,2,8),PRIs6[[-1,-2]]], 'NP':[[],[15],[]], 'PRNP':(2,'seq') }
rspec['etsi-6-2abab'] = { 'PW':[[0.8],[1.4],[2]], 'PRI':[PRIs6[[0,1]],midN(PRIs6,2,8),PRIs6[[-1,-2]]], 'NP':[[],[15],[]], 'PRNP':(2,'alt') }

rspec['etsi-6-3aabb'] = { 'PW':[[0.8],[1.4],[2]], 'PRI':[PRIs6[[0,1,2]],midN(PRIs6,3,8),PRIs6[[-1,-2, -3]]], 'NP':[[],[15],[]], 'PRNP':(3,'seq') }
rspec['etsi-6-3abab'] = { 'PW':[[0.8],[1.4],[2]], 'PRI':[PRIs6[[0,1,2]],midN(PRIs6,3,8),PRIs6[[-1,-2, -3]]], 'NP':[[],[15],[]], 'PRNP':(3,'alt') }

rspec['etsi-stag2-not'] = { 'PW':[0.5, 0.7,],'PRI':[50,70], 'NP':[5], 'PRNP':(2,'alt') }
rspec['etsi-stag3-not'] = { 'PW':[0.5, 0.7, 0.6],'PRI':[50,70,90], 'NP':[5], 'PRNP':(3,'alt') }


# Taken from:
# http://hwnbu-twiki.sj.broadcom.com/twiki/pub/Mwgroup/DFSMainPage/japan_061004_1_si06.pdf
#
rspec['jp-1-1'] = { 'PW':[[],[1.0],[]],'PRI':[[],[int(1e6/700)],[]], 'NP':[[],[18],[]] }
rspec['jp-1-2'] = { 'PW':[[],[2.5],[]],'PRI':[[],[int(1e6/260)],[]], 'NP':[[],[18],[]] }

rspec['jp-2-1-1'] = { 'PW':[[],[0.5],[]],'PRI':[[],[int(1e6/720)],[]], 'NP':[[],[18],[]] }
rspec['jp-2-1-2'] = { 'PW':[[],[2.0],[]],'PRI':[[],[int(1e6/250)],[]], 'NP':[[],[18],[]] }

# Not sure about these
# Seem like esti - took mid range PW, PRF and NP values
rspec['jp-2-2-1'] = { 'PW':[[],[1.0],[]],     'PRI':[[],[int(1e6/700)],[]], 'NP':[[],[18],[]] }
rspec['jp-2-2-2'] = { 'PW':[[1.],[3.0],[5.]],   'PRI':[[int(1e6/6666)],[int(1e6/5000)],[int(1e6/4348)]], 'NP':[[23],[26],[29]] }
rspec['jp-2-2-3'] = { 'PW':[[6.],[8.0],[10.]],  'PRI':[[int(1e6/4000)],[int(1e6/3000)],[int(1e6/2000)]], 'NP':[[16],[17],[18]] }
rspec['jp-2-2-4'] = { 'PW':[[11.],[15.5],[20.]],'PRI':[[int(1e6/4000)],[int(1e6/3000)],[int(1e6/2000)]], 'NP':[[12],[14],[16]] }
# Not sure about these ones as cannot interpret the diagrams
# rspec['fcc-5'] = { 'PW':[[50],[75],[100]],'PRI':[[1000],[1500],[2000]], 'NP':[[1,2],[3],[2,3]], 'FM':[510], 'BRI':[int(12./20.*1e6)]  }
rspec['jp-3-1'] = { 'PW':[[50.],[75],[100.]],   'PRI':[[int(1e6/1000)],[int(1e6/750)],[int(1e6/500)]], 'NP':[[1,2],[3],[2,3]], 'FM' : [510], 'BRI':[int(12./20.*1e6)]  }
rspec['jp-4-1'] = { 'PW':[[],[1.0],[]],         'PRI':[[],[int(1e6/3000)],[]], 'NP':[[],[9],[]],  'BRI':[0.3e6/100] }  # 100 bursts in 0.3sec??

rspec['jp-not'] = { 'PW':[150],'PRI':[int(1e6/100)], 'NP':[5] }


# kor-1 Same as fcc-0
rspec['kor-1'] = rspec['fcc-0']# { 'PW':[[],[1],[]],     'PRI':[[],FCCTable5ATestA_PRI[11:12],[]],  'NP':[[],FCCTable5ATestA_NP[11:12],[]]  }
rspec['kor-2'] = { 'PW':[[],[1],[]],     'PRI':[[],[556],[]],  'NP':[[],[10],[]] }
rspec['kor-3'] = { 'PW':[[],[2],[]],     'PRI':[[],[3030],[]], 'NP':[[],[70],[]], 'BRI':[int(12./20.*1e6)]  } # Use long BRI for long PRI
rspec['kor-4'] = { 'PW':[[],[1],[]],     'PRI':[[],[333],[]],  'NP':[[],[0,0,0,3],[]] } # emulate freq hopping by sending out a 0 pulse burst



pulsedesc = np.dtype([('T0', 'uint32'), ('T2', 'uint16'), ('FM', 'uint16') , ('FC', 'uint16'), ('CHIRP', 'uint16'), ('TIMENOTRADAR', 'uint16') ])
pulses = np.zeros(shape=(maxpulses,), dtype=pulsedesc )
fifo = np.zeros(shape=(maxpulses, wordsperpulse), dtype=np.dtype('uint16'))

bw = 20
ant = 0
rlog = open('%s-gen.log' %fprefix, 'wb')



rdrs = rspec.keys()
rdrs.sort()

# corners for PW PRI NP
# 0 == min  1 = mid  2 == max
cornerVars = [ 'PW', 'PRI', 'NP']
corners = {
   'max': [2,0,0],
   'min': [2,2,0],
}

rlog.write('# $ Copyright Broadcom Corporation $\n')
rlog.write('#\n')
rlog.write('#\n')
rlog.write('# <<Broadcom-WL-IPTag/Proprietary:.*>>\n')
rlog.write('#\n')
rlog.write('# $Id$\n')

# Pretty print the rspec
#  radar   PW[3]  PRI[3]  NP[3]   PRNP  FM BRI
#
rlog.write('RADAR SPECS(CSV FORMAT),,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,\n')
rlog.write('radar, ,PW,,,  ,PRI,,, ,NP,,,  ,PRNP,,,  ,FM,,,  ,BRI,,,\n')
rlog.write('     , ' +'min,mid,max,,'*6 + '\n' )
for r in rdrs:
   rlog.write('%s,' %r)
   for f in ['PW', 'PRI', 'NP', 'PRNP',  'FM', 'BRI']:
      if f in rspec[r].keys():
          if f in cornerVars and '-not' not in r:
              for i in range(3):
                  if len(rspec[r][f][i]) == 1:
                      rlog.write('%s,' %(str(rspec[r][f][i][0]).translate(None,'[]').replace(',',';')))
                  else:
                      rlog.write('%s,' %(str(rspec[r][f][i])).translate(None,'[]').replace(',',';'))
              rlog.write(',' )
          else:
              #rlog.write('-,%s,-,,' %str(rspec[r][f]).replace(' ','').replace(',','/').replace('(','').replace(')',''))
              rlog.write('-,%s,-,,' %str(rspec[r][f]).translate(None,'[]()').replace(',',';'))
      else:
          rlog.write('-,-,-,,')
   rlog.write("\n")
rlog.write(',,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,\n')

radarsgen = {}


for r in rdrs:
    # copy typical or mid range values over
    radarsgen[r]= copy.deepcopy(rspec[r])
    # only take mid vaules for PW PRI NP
    if '-not' not in r:
        radarsgen[r]['PW'] = rspec[r]['PW'][1]
        radarsgen[r]['PRI'] = rspec[r]['PRI'][1]
        radarsgen[r]['NP'] = rspec[r]['NP'][1]

# Generate corner cases based on above table
# Check that corner index makes sense - else use mid value
# Generate if at least one var is different from mid
if 1:
 for r in rdrs:
    if '-not' not in r:
        for c in corners:
            validcorner = False
            cnrr = copy.deepcopy(rspec[r])
            for f in cornerVars:
                ci = corners[c][cornerVars.index(f)]
                #print r, c, ci
                if rspec[r][f][ci] <> []:
                    validcorner = True
                    cnrr[f] = rspec[r][f][ci]
                else:
                    cnrr[f] = rspec[r][f][1]

            if validcorner:
                radarsgen['zcorner-%s-%s' %(c,r)] = cnrr
            else:
                x = "Skipped corner '%s' as no valid case for radar '%s'" %(c,r)
                print x
                rlog.write( x +"\n")


rdrs = radarsgen.keys()
rdrs.sort()

for r in rdrs:
    rlog.flush()
    rlog.write("%s %s\n" %(r,radarsgen[r]))

for r in rdrs:
 if 'seed' in radarsgen[r].keys():
  random.seed(int(radarsgen[r]['seed']))
  np.random.seed(int(radarsgen[r]['seed']))
 else:
  random.seed(options.seed)
  np.random.seed(options.seed)
 for bw in [ 20, 40, 80]:
   if options.version == 1:
       bwk = (bw/20)
   else:
       bwk = 1
   # Ensure near identical pulse patterns are seen by each antenna
   STAG_N = 1
   STAG_PAT = ''
   # Determine number of different PRF to use
   if 'PRNP' in radarsgen[r].keys():
    STAG_N = radarsgen[r]['PRNP'][0]
    STAG_PAT = radarsgen[r]['PRNP'][1]
    STAGPRI = random.sample(radarsgen[r]['PRI'], STAG_N)
   else:
    PRI = random.choice(radarsgen[r]['PRI'])
   # toa in units of time. 
   # Is converted to clock counts in 'T0'
   # FIXME: No mechanism/testing of/for overflow 
   # Allow 12 bits for scaling to clock counts and num_pulses*PRI/BRI
   toa_start = np.random.randint(1<<20)
   print r,"0x%x" %toa_start
   for ant in range(2):
    # print r,bw,ant
    burst_start = toa = toa_start
    T0 = []
    T2 = []
    FM = []
    # NEW measurements  4366c0 160MHz ++
    FC = []  
    CHIRP = []
    TIMENOTRADAR = []
    fc = 0x012
    chirp =  9 # FIXME Temporary values
    timenotradar = 500 #  FIXME Temporary values
    #
    pcnt = 0
    PRF_CNT = 0
    PW = random.choice(radarsgen[r]['PW'])
    NP = random.choice(radarsgen[r]['NP']) # Number of pulses per PRF
    while (len(T0) < maxpulses): # Fill up buffer
      if NP <> 0:
        # FW filters pulses  in range[0..206]
        # FM chirp radars(bin5) filter by range(450??..511)
        if 'FM' in radarsgen[r].keys():
          fm = int(random.choice(radarsgen[r]['FM'])) # FM det tends to limit. Scaling non-sensical.
        else:
          fm = np.random.randint(257,400) # FW filters pulses  if  '< 0' or (uint)206-256. Now 1..114  
          #fm = np.random.randint(206,256) # FW filters pulses  if  '< 0' or (uint)206-256.
        # TONEDETECTION = 1 always so table values are offset 256
        if STAG_PAT== 'alt':
          PRI = STAGPRI[pcnt % STAG_N]
        elif STAG_PAT== 'seq':
          PRI = STAGPRI[pcnt / NP ]
        elif STAG_PAT== '':
          pass
        else:
          print "ERROR: STAG_PAT? for %s" %r
          exit(-1)
        T0.append(toa*20*bwk + bwk*np.random.randint(-noise,noise+1)) # assume sampling at BW(MHz)
        T2.append(PW*20*bwk + bwk*np.random.randint(-noise,noise+1))
        FM.append(fm)
        # NEW measurements  4366c0 160MHz ++
        FC.append(fc)
        CHIRP.append(chirp)
        TIMENOTRADAR.append(timenotradar)

        # rlog.write( "%s: TOA: %d,Pulse: %d, PW: %d, PRI: %d, NP: %d, FM: %d\n" %(r, toa,pcnt, PW,PRI, NP,fm) )
        pcnt += 1
        toa += PRI
      # check at end of burst
      if pcnt == NP*STAG_N:
        if 'BRI' in radarsgen[r].keys():
          BRI = random.choice(radarsgen[r]['BRI'])
        else:
          BRI = BRI_default

        if (toa - PRI + PW - burst_start) >= BRI:
          print "ERROR Bursts too close together %s %d %d curr burst duration:%d BRI %d" %(r,bw,ant,toa - PRI + PW-burst_start, BRI)
          exit(-1)
        else:
          toa = burst_start + BRI # next burst offset from first
        # track current burst toa start
        burst_start = toa
        pcnt = 0
        # change PW and NP in the burst
        PW = random.choice(radarsgen[r]['PW'])
        NP = random.choice(radarsgen[r]['NP']) # Number of pulses per PRF
        #print "%s: added BRI" %r
      #print "%s NP: %d pcnt: %d " %(r, NP, pcnt)
    pulses['T0'] = np.array(T0, dtype=np.dtype('uint32'))
    pulses['T2'] = np.array(T2, dtype=np.dtype('uint16'))
    pulses['FM'] = np.array(FM, dtype=np.dtype('uint16'))
    # New stuff version 2
    pulses['FC'] = np.array(FC, dtype=np.dtype('uint16'))
    pulses['CHIRP'] = np.array(CHIRP, dtype=np.dtype('uint16'))
    pulses['TIMENOTRADAR'] = np.array(TIMENOTRADAR, dtype=np.dtype('uint16'))
    #
    ff = open( "%s_%s_bw%d_ant%d.bin" %(fprefix,r,bw,ant), 'wb' )
    # pack fifo using T0 T2 and FM
    # (bits High to Low)
    # First Word:	Time0[31:16]
    # Second Word:	Time2[3:0] | Time0[15:4]
    # Third Word:	FMDiff[7:0] | Time2[11:4]
    # Fourth Word:	FMDiff[8] | Time2[12] | Time0[3:0]
    fifo[:,0] = (pulses['T0'] & 0xffff0000) >> 16
    fifo[:,1] = (pulses['T2'] & 0xF )<<12 | (pulses['T0'] & 0xFFF0) >> 4
    fifo[:,2] = (pulses['FM'] & 0xFF )<<8 | (pulses['T2'] & 0xFF0) >> 4
    if options.version == 1:
        fifo[:,3] = (pulses['FM'] & 0x100 )>>3 | (pulses['T2'] & 0x1000) >> 8 | (pulses['T0'] & 0xF)
    elif options.version == 2:
        fifo[:,3] = (pulses['FM'] & 0x100 )>>3 | (pulses['T2'] & 0x1000) >> 8 | (pulses['T0'] & 0xF) | (pulses['CHIRP'] & 0x1FF) << 6
        fifo[:,4] =  pulses['FC'] & 0x01FF
        fifo[:,5] =  (pulses['TIMENOTRADAR'] & 0x01FFF) << 3
    else:
        print "Unknown version to pack into FIFO" # Should never get here anyway!
        exit()
    ff.write(np.ravel(fifo).tostring())
    ff.close()
rlog.close()
print "Success!!"
