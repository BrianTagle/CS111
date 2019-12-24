#!/bin/bash
#NAME: Brian Tagle
#EMAIL: 604907076
#ID: 604907076

errors=0
./lab4b --period=5 --scale=F --log=TESTLOG <<-EOF > /dev/null
SCALE=C
PERIOD=1
STOP
START
LOG test
OFF
EOF
for ret in "SCALE=C" "PERIOD=1" STOP START LOG test OFF
do
    grep $ret TESTLOG > /dev/null
    if [[ $? -eq 0 ]]
    then
	echo "SUCCESS: $ret found in TESTLOG"
    else
	echo "FAILURE: $ret not found in TESTLOG"
	errors+=1
    fi
done
rm -f TESTLOG
if [[ "$errors" -eq 0 ]]
then
    echo "PASSED SMOKE CHECK"
else
    echo "FAILED SMOKE CHECK"
fi
