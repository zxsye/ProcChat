#!/bin/bash

inputs=(testcases/bin/*)
passed=0
total_test=0

for infile in "${inputs[@]}";
do
    echo ""
    echo "TEST: ${infile}"
    # set outfile name
    outfile=${infile/\/bin\//\/out\/}
    outfile+=".out"

    # Run test cases
    ./procchat & 
    sleep 1 &
    ./$infile | diff - $outfile

    # Check test result
    if [ $? -eq 0 ]; then
        echo "###############"
        echo "#### PASS! ####"
        echo "###############
        "
        passed=$((passed+1))
        total_test=$((total_test+1))

    elif [ $? -eq 1 ]; then
        echo "@ @ @ @ @ @ @ @"
        echo " @ @ FAIL! @ @ "
        echo "@ @ @ @ @ @ @ @
        "
        total_test=$((total_test+1))
    fi

    # kill $(ps aux | grep '[p]rocchat' | awk '{print $2}')
    pkill -9 procchat
    pkill -9 $infile
    
    rm gevent

done

echo "#######################################"
echo "####### Passed $passed/$total_test test cases #######"
echo "#######################################
"

rm -rf disconnect
rm -rf disconnect_multidom
rm -rf disconnect_multidom_2
rm -rf multi_multiconnect_1
rm -rf multi_multiconnect_2
rm -rf multi_SAY
rm -rf multi_SAY_2
rm -rf multi_SAYCONT
rm -rf multi_SAYCONT_2
rm -rf one_connect
rm -rf one_multiconnect
rm -rf one_SAY
rm -rf one_SAYCONT
rm -rf one_SAYCONT_int
rm -rf ping_pong
rm -rf faulty_message
rm -rf nested
rm -rf one_connect_again
rm -rf SAY_SAYCONT_long