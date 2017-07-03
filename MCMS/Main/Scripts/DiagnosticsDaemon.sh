#!/bin/sh

DIAGNOSTICS=Bin/Diagnostics

cnt=0
while [ 1 ] 
do 
  cnt=`expr $cnt + 1`;
  echo "Run Diagnostic $cnt" ; 
  $DIAGNOSTICS ;
  sleep 80 ;
done 
