#!/bin/sh

#clean cs loader

CS_PID=$(ps -ef | grep acloader | grep -v ' grep ' | awk '{print $2}') 

kill $CS_PID

Scripts/Cleanup.sh

#clean calltask
CALL_TASK=$(ps -ef | grep calltask |grep -v ' grep '|awk '{print $2}')

for task in $CALL_TASK
do
  echo "kill " $task
  kill -9 $task
done

#clean mcmsif

MCMSIF_PID=$(ps -ef | grep mcmsif | grep -v ' grep ' | awk '{print $2}')

kill -9 $MCMSIF_PID

#clean csman

CSMAN_PID=$(ps -ef | grep csman | grep -v ' grep ' | awk '{print $2}')

kill -9 $CSMAN_PID

#clean  h323LoadBalancer
LOADBALANCER_PID=$(ps -ef | grep h323LoadBalancer | grep -v ' grep ' | awk '{print $2}')

kill -9 $LOADBALANCER_PID

#clean gkiftask
GKIFTASK_PID=$(ps -ef | grep gkiftask | grep -v ' grep ' | awk '{print $2}')

kill -9 $GKIFTASK_PID


#clean siptask
SIPTASK_PID=$(ps -ef | grep siptask | grep -v ' grep ' | awk '{print $2}')

kill -9 $SIPTASK_PID

