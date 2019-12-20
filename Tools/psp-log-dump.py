#!/usr/bin/env python3
import csv;
import sys;
import os;
import struct;

from enum import Enum;

g_PspMarker1 = 0xd00f;
g_PspMarker2 = 0xf00d;
g_PspMarker3 = 0xfeed;
g_PspMarker4 = 0xfade;
g_DbgLogMarker = 0xdead;

class PspState(Enum):
    DETECTING_1 = 1
    DETECTING_2 = 2
    DETECTING_3 = 3
    IDLE        = 2 # Waiting for a debug marker
    DBG_LOG     = 3 # Dumping a debug log

class Psp(object):
    """
    The class representing a single PSP.
    """

    def __init__(self, idPsp, idDbgMarker):
        self.idPsp       = idPsp;
        self.idDbgMarker = idDbgMarker;
        self.enmState    = PspState.DETECTING_1;
        self.sLog        = '';

    def getId(self):
        return self.idPsp;

    def getState(self):
        return self.enmState;

    def setState(self, enmState):
        self.enmState = enmState;

    def appendToLog(self, sAppend):
        self.sLog += sAppend;

class PspLogExtractor(object):
    """
    The main class handling log extraction from CSV exported logic traces.
    """

    def __init__(self):
        self.sToolName        = None;
        self.sInputCsv        = None;
        self.sOutputLog       = None;
        self.dPspDetected     = { };  # The dictionary containing the detected PSPs keyed by ID

    def showUsage(self):
        """
        Prints the usage of the tool to stdout.
        """
        print('%s Options:' % (self.sToolName,));
        print('  --input-csv       <path to logic trace in CSV format>');
        print('      The path to the logic trace in CSV format');
        print('  --output          <extracted log path>');
        print('      Where to store the extracted log');

    def parseOption(self, asArgs, iArg):
        """
        Parses a single option at the given index.
        """
        if asArgs[iArg] == '--input-csv':
            iArg += 1;
            if iArg >= len(asArgs): raise Exception('Invalid option', '--input-csv takes a file path');
            self.sInputCsv = asArgs[iArg];
        elif asArgs[iArg] == '--output':
            iArg += 1;
            if iArg >= len(asArgs): raise Exception('Invalid option', '--output takes a file path');
            self.sOutputLog = asArgs[iArg];
        else:
            self.showUsage();
            raise Exception('Invalid option', 'Option "%s" is unknown' % (asArgs[iArg],));

        return iArg + 1;

    def main(self, asArgs = None):
        """
        Main entry point doing the argument parsing and doing the work.
        """

        self.sToolName = asArgs[0];
        iArg = 1;
        try:
            while iArg < len(asArgs):
                iNext = self.parseOption(asArgs, iArg);
                if iNext == iArg:
                    self.showUsage();
                    raise Exception('Invalid option', 'Option "%s" is unknown' % (asArgs[iArg],));
                iArg = iNext;
        except Exception as oXcpt:
            print(oXcpt);
            sys.exit(1);

        # Check that all required options present.
        if    self.sInputCsv is None\
           or self.sOutputLog is None:
            print('A required option is missing');
            self.showUsage();
            sys.exit(1);

        # Load the CSV
        oInputCsv = open(self.sInputCsv);
        oCsvRdr = csv.reader(oInputCsv, delimiter=',');

        # Extract the MOSI lines and convert to byte array
        # @todo Improve and don't require to create a big chunk of memory.
        sRes = ' '.join(sLine[2].strip()[2:] for sLine in oCsvRdr);
        abBytes = bytearray.fromhex(sRes[3:]);

        oInputCsv.close();
        oLogOut = open(self.sOutputLog + '.raw', "wb");
        oLogOut.write(abBytes);
        oLogOut.close();

        # Traverse bytes and look for a debug marker.
        idxBytes = 0;
        while idxBytes < len(abBytes):
            if idxBytes + 4 >= len(abBytes):
                break;

            uMarker, uPspId = struct.unpack('<HH', abBytes[idxBytes:idxBytes+4]);
            if uMarker == g_DbgLogMarker:
                oPsp = self.dPspDetected.get(uPspId);
                if oPsp is not None:
                    # If the PSP is idling set to accepting debug out and vice versa.
                    if oPsp.getState() == PspState.DBG_LOG:
                        oPsp.setState(PspState.IDLE);
                        idxBytes += 4;
                    elif oPsp.getState() == PspState.IDLE:
                        oPsp.setState(PspState.DBG_LOG);
                        idxBytes += 4;
                    else:
                        idxBytes += 1;
                        #print('Invalid PSP device state!!!');
                else:
                    idxBytes += 1;
            elif uMarker == g_PspMarker1:
                # Looks like a new PSP came up, check that the ID does not exist and create a new PSP instance
                oPsp = self.dPspDetected.get(uPspId);
                if oPsp is None:
                    idPspNew = len(self.dPspDetected);
                    oPsp = Psp(idPspNew, uPspId);
                    self.dPspDetected[uPspId] = oPsp;
                    idxBytes += 4;
                    print('Detected a PSP %s with debug marker ID %s' % (idPspNew, uPspId));
                else:
                    idxBytes += 1;
                    print('The PSP with ID %s is already known, reset happening?' % (uPspId, ));
            elif uMarker == g_PspMarker2:
                oPsp = self.dPspDetected.get(uPspId);
                if     oPsp is not None \
                   and oPsp.getState() == PspState.DETECTING_1:
                    oPsp.setState(PspState.DETECTING_2);
                    idxBytes += 4;
                else:
                    idxBytes += 1;
            elif uMarker == g_PspMarker3:
                oPsp = self.dPspDetected.get(uPspId);
                if     oPsp is not None \
                   and oPsp.getState() == PspState.DETECTING_2:
                    oPsp.setState(PspState.DETECTING_3);
                    idxBytes += 4;
                else:
                    idxBytes += 1;
            elif uMarker == g_PspMarker4:
                oPsp = self.dPspDetected.get(uPspId);
                if     oPsp is not None \
                   and oPsp.getState() == PspState.DETECTING_3:
                    print('Fully initialized PSP %s with debug marker ID %s' % (oPsp.getId(), uPspId));
                    oPsp.setState(PspState.IDLE);
                    idxBytes += 4;
                else:
                    idxBytes += 1;
            else:
                # Check whether the PSP ID is known and the PSP currently outputting debug information
                # If not discard the candidate and move on.
                oPsp = self.dPspDetected.get(uPspId);
                if     oPsp is not None \
                   and oPsp.getState() == PspState.DBG_LOG:
                    # Append data to debug log
                    if (uMarker & 0xff) != 0:
                        oPsp.appendToLog(str(chr(uMarker & 0xff)));
                    if (uMarker >> 8) != 0:
                        oPsp.appendToLog(str(chr((uMarker >> 8) & 0xff)));
                    idxBytes += 4;
                else:
                    idxBytes += 1;

            if idxBytes % (1024 * 1024) == 0:
                print('Processed %s out of %s bytes' % (idxBytes, len(abBytes)));

        oLogOut = open(self.sOutputLog, "w");

        for oPsp in self.dPspDetected.values():
            oLogOut.write('\n\n\n');
            oLogOut.write('Log of PSP %s: \n' % (oPsp.getId(), ));
            oLogOut.write(oPsp.sLog);
        oLogOut.close();

        sys.exit(0);


if __name__ == '__main__':
    sys.exit(PspLogExtractor().main(sys.argv));
