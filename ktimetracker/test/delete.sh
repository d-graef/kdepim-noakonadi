#!/bin/sh

# check if deleting a task works

TESTFILE="/tmp/testkarm1.ics"
rm $TESTFILE

source __lib.sh

set_up

# do not prompt on deleting a task
GPOD=`dcop $DCOPID KarmDCOPIface getpromptdelete 2>/dev/null`
RVAL=`dcop $DCOPID KarmDCOPIface setpromptdelete 0 2>/dev/null`

RVAL=`dcop $DCOPID KarmDCOPIface addtodo todo1 2>/dev/null`
RVAL=`dcop $DCOPID KarmDCOPIface addtodo todo2 2>/dev/null`
RVAL=`dcop $DCOPID KarmDCOPIface deletetodo 2>/dev/null`
RVAL=`dcop $DCOPID KarmDCOPIface deletetodo 2>/dev/null`
RVAL=`dcop $DCOPID KarmDCOPIface setpromptdelete $GPOD 2>/dev/null`
RVAL=`dcop $DCOPID KarmDCOPIface version 2>/dev/null`

tear_down

if [ "$RVAL" == "" ]; then 
  echo "FAIL $0: got no version."
  exit 0;
else 
  echo "PASS $0"
  exit 1;
fi
