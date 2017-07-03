#!/bin/sh

#	enviroment exports  
export CS_SIMULATION_TEST=YES

export GIDEONSIM_MNGMNT_CONFIG_FILE=Cfg/NetworkCfg_Management.xml

export USE_ALT_IP_SERVICE=Scripts/CsSimulationConfig/IPServiceList_default_udp.xml

export USE_ALT_MNGMNT_SERVICE=Scripts/CsSimulationConfig/NetworkCfg_Management_ipv4_only.xml

export ENDPOINTSSIM=NO

# prepare cs
./Scripts/make_cs_simulation_env.py

# launch cs
 cd /cs
  cp /mcms/KeysForCS/cs1/* ocs/cs1/ 
   echo "after copying certificates "
   ls -l /cs/ocs/cs1    
  ./bin/loader > /cs/logs/output/cs_console.txt&
 cd -
sleep 2
ls -l /cs/ocs/cs1
# launch MCMS
 ./Scripts/Startup.sh

