#!/bin/bash
# CollectInfo.sh
#
# Written by David Rabkin
# <david.rabkin@polycom.co.il>

# Edit following parameteres.
START_TIME="2013-01-01T00:00:00"
END_TIME="2033-01-01T00:00:00"
COREDUMPS="true"

OUTP="$MCU_HOME_DIR/tmp/result.out"
AUTH="___AUTH_TO_REPLACE___"
TYPE="Content-Type: application/vnd.plcm.dummy+xml"
PARM="If-None-Match: -1"
SOCKET=`grep '^Listen' $MCU_HOME_DIR/tmp/httpd.listen.conf | head -1 | cut -d' ' -f2`
IP=`echo -n $SOCKET | cut -d':' -f1`
PORT=`echo -n $SOCKET | cut -d':' -f2`
DATA="<COLLECT_INFO><START_TIME>$START_TIME</START_TIME><END_TIME>$END_TIME</END_TIME><COLLECT_INFO_LIST><COLLECT_TYPE><TYPE>collecting_type_audit</TYPE><MARK_FOR_COLLECTION>false</MARK_FOR_COLLECTION></COLLECT_TYPE><COLLECT_TYPE><TYPE>collecting_type_cdr</TYPE><MARK_FOR_COLLECTION>false</MARK_FOR_COLLECTION></COLLECT_TYPE><COLLECT_TYPE><TYPE>collecting_type_cfg</TYPE><MARK_FOR_COLLECTION>true</MARK_FOR_COLLECTION></COLLECT_TYPE><COLLECT_TYPE><TYPE>collecting_type_core_dumps</TYPE><MARK_FOR_COLLECTION>$COREDUMPS</MARK_FOR_COLLECTION></COLLECT_TYPE><COLLECT_TYPE><TYPE>collecting_type_faults_logs</TYPE><MARK_FOR_COLLECTION>true</MARK_FOR_COLLECTION></COLLECT_TYPE><COLLECT_TYPE><TYPE>collecting_type_logs</TYPE><MARK_FOR_COLLECTION>true</MARK_FOR_COLLECTION></COLLECT_TYPE><COLLECT_TYPE><TYPE>collecting_type_processes_info</TYPE><MARK_FOR_COLLECTION>true</MARK_FOR_COLLECTION></COLLECT_TYPE><COLLECT_TYPE><TYPE>collecting_type_network_traffic_capture</TYPE><MARK_FOR_COLLECTION>true</MARK_FOR_COLLECTION></COLLECT_TYPE><COLLECT_TYPE><TYPE>collecting_type_participants_recordings</TYPE><MARK_FOR_COLLECTION>false</MARK_FOR_COLLECTION></COLLECT_TYPE><COLLECT_TYPE><TYPE>collecting_type_nids</TYPE><MARK_FOR_COLLECTION>false</MARK_FOR_COLLECTION></COLLECT_TYPE></COLLECT_INFO_LIST></COLLECT_INFO>"

IS_SECURE=`echo -n \`cat $MCU_HOME_DIR/mcms/Cfg/NetworkCfg_Management.xml | grep '<IS_SECURED>true</IS_SECURED>'\``

if [[ "$IS_SECURE" != ""  &&  "$PORT" == "443" ]]; then
        HTTP_PREF="https://$IP"
elif [ "$IS_SECURE" != "" ]; then
        HTTP_PREF="https://$IP:$PORT"
else
        HTTP_PREF="http://$IP:$PORT"
fi



# Starts the process with a certain object that describes types of requested
# files and a time frame.
curl -1 -s -k -H "$PARM" -H "$TYPE" -H "$AUTH" -X POST -d "$DATA" ${HTTP_PREF}/plcm/mcu/api/1.0/system/info_collector/pack > "$OUTP"

#if ! grep -q "Status OK" "$OUTP"
if [ ! $? -eq 0 ]
then
  echo -e "\e[0;31mFailed\e[0m to start"
  cat "$OUTP"
  echo ""
  rm "$OUTP"
  exit 1
fi

echo -n "Collecting (point per 10 seconds)"

# Timeout is 1 hour.
for i in {1..360}
do
  echo -n "."
  sleep 10

  # Probes the collecting process status and verify the process is finished.
  # If it is not wait for 2 seconds and probe again.
 
  curl -1 -s -k -H "$PARM" -H "$TYPE" -H "$AUTH" -G ${HTTP_PREF}/plcm/mcu/api/1.0/system/info/state > "$OUTP"

  if grep -q "<COLLECTING_INFO>0</COLLECTING_INFO>" "$OUTP"
  then
    echo -e "\e[0;32mok\e[0m"
    break
  fi

done

# Probes once again to define quit by timeout.

curl -1 -s -k -H "$PARM" -H "$TYPE" -H "$AUTH" -G ${HTTP_PREF}/plcm/mcu/api/1.0/system/info/state > "$OUTP"

if ! grep -q "<COLLECTING_INFO>0</COLLECTING_INFO>" "$OUTP"
then
  echo -e "\e[0;31mtimeout\e[0m"
  cat "$OUTP"
  echo ""
  rm "$OUTP"
  exit 2
fi

# Gets a name of a resulting file.

curl -1 -s -k -H "$PARM" -H "$TYPE" -H "$AUTH" -G ${HTTP_PREF}/plcm/mcu/api/1.0/system/info_collector/pack > "$OUTP"

echo -n "Generated file is $MCU_HOME_DIR/mcms/"
sed -n -e 's/.*<LAST_GENERATED_FILE>\(.*\)<\/LAST_GENERATED_FILE>.*/\1/p' "$OUTP"
rm "$OUTP"

