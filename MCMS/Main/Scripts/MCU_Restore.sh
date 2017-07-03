#!/bin/bash
###################################
# MCU_Restore.sh
#
# Written by Shachar Bar
# <shachar.bar@polycom.co.il>
###################################
Help() 
{
	if [[ "$PRODUCT_TYPE" == "SOFT_MCU_MFW" ]] ; then		
		echo "Usage: $0 Backup_file -u <User Name> -p <User Password>"
	else
		echo "Usage: $0 Backup_file [ -u <User Name> -p <User Password> ]"
	fi


	exit 1	
}
PRODUCT_TYPE=`cat $MCU_HOME_DIR/mcms/ProductType`
#Get User Password if exist
args=`getopt u:p: $*`
set -- $args

user=""
password=""

for i; do
  case "$i" in
    -u  ) user="$2"
          shift ; shift  ;;
    -p  ) password="$2"
         shift ; shift  ;;
    --  ) shift; break ;;
  esac
done

if [[ "X$user" == "X" || "X$password" == "X" ]]; then
	if [[ "$PRODUCT_TYPE" == "SOFT_MCU_MFW" ]] ; then
		Help  
	else
		user="SUPPORT"
		password="SUPPORT"
	fi
fi

UserPassword="$user:$password"

AUTH=`echo -n $UserPassword | openssl base64`

AUTH="Authorization: Basic $AUTH"

#echo "user: $user password $password AUTH $AUTH"



MCU_HOME_DIR=
MCU_HOME_DIR=$(grep MCU_HOME_DIR /home/mcms/.bashrc | cut -d'=' -f2 | awk 'NR==1')
export MCU_LIBS=$MCU_HOME_DIR/lib:$MCU_HOME_DIR/lib64:$MCU_HOME_DIR/usr/lib:$MCU_HOME_DIR/usr/lib64

# Variable definitions:
OUTP="$MCU_HOME_DIR/tmp/result_restore.out"
OUTHEADER="$MCU_HOME_DIR/tmp/header_restore.out"
UPLOAD_TYPE="Content-Type: multipart/form-data"
TYPE="Content-Type: application/vnd.plcm.dummy+xml"
PARM="If-None-Match: -1"
EMA_PORT=`grep '^Listen' $MCU_HOME_DIR/tmp/httpd.listen.conf | cut -d' ' -f2 | cut -d':' -f2 | head -1`
MYIP=`netstat -ntl | grep -E "[0-9]+\:${EMA_PORT}[^0-9]" | tr -s ' ' | cut -d' ' -f4 | cut -d':' -f1`
Restore_Original_Location="$MCU_HOME_DIR/mcms/Restore"
Restore_File=$1
Simulation=$2

SOCKET=`grep '^Listen' $MCU_HOME_DIR/tmp/httpd.listen.conf | head -1 | cut -d' ' -f2`
IP=`echo -n $SOCKET | cut -d':' -f1`
PORT=`echo -n $SOCKET | cut -d':' -f2`
IS_SECURE=`echo -n \`cat $MCU_HOME_DIR/mcms/Cfg/NetworkCfg_Management.xml | grep '<IS_SECURED>true</IS_SECURED>'\``





if [[ "$IS_SECURE" != ""  &&  "$PORT" == "443" ]]; then
        HTTP_PREF="https://$IP"
elif [ "$IS_SECURE" != "" ]; then
        HTTP_PREF="https://$IP:$PORT"
else
        HTTP_PREF="http://$IP:$PORT"
fi



echo "Protocol: $HTTP_PREF"


export Restore_File

##################################
# Interrupt Handler 
##################################
Interrupt() {
	echo "Backup operation was interrupted by the user"
	curl -1 -s -k -H "$PARM" -H "$AUTH" -F file=@${Restore_File} ${HTTP_PREF}/plcm/mcu/api/1.0/system/restore/finish/`basename ${Restore_File}` > "$OUTP"
	rm -f ${Restore_Original_Location}/Backup*.bck
	rm -f $OUTP
	exit 5
}

IsNoContentOrOK()
{
        noContentExist=`grep  "204 No Content" $OUTHEADER`
        if [[ $noContentExist =~ "204 No Content" ]] ;
        then
                return 0
        fi
        okContentExist=`grep  "200 OK" $OUTHEADER`
        if [[ $okContentExist =~ "200 OK" ]] ;
        then
                return 0
        fi
        
        return 1
}

##################################
# Interrupt during backup operaiton
trap Interrupt INT TERM


# Checking for the backup file
if [[ "X$1" == "X" ]]; then
	echo "Missing backup file"
	Help
fi

# Checking for valid backup file
if [[ ! -r $1 ]]; then
	echo "Backup file $1 doesn't exist"
	Help
fi

# Making sure /mcu_custom_config is with mcms ownership
if [[ $? > 0 ]]; then
	if [[ "$Simulation" == "YES" ]]; then
		echo "Simulation Env please make sure /mcu_custom_config folder exists otherwize Restore might Fail!"
	else
		chown -R mcms.mcms /mcu_custom_config
		echo "Preparing /mcu_custom_config to be restored failed"
		exit 7
	fi
fi

rm -f $OUTP
rm -f $OUTHEADER


#Fix BRIDGE-13005
cp ${Restore_File} $Restore_Original_Location
chmod 777 $Restore_Original_Location/`basename ${Restore_File}`

# Staring the restore operation
echo "Restore operation started"
curl -1 -s -k -H "$PARM" -H "$TYPE" -H "$AUTH" -X POST ${HTTP_PREF}/plcm/mcu/api/1.0/system/restore/start > "$OUTP" --dump-header "$OUTHEADER" 2> /dev/null

# Test if the MCU service is up. Curl will return an empty file if it cannnot connect.
if ! IsNoContentOrOK ; then
	echo "There was a problem connecting to the MCU service: $(<$OUTP)"
	exit 3
fi

rm -f $OUTP
rm -f $OUTHEADER

# Uploading backup file 
echo "Uploading the restore file"

curl -1 -s -k -H "$PARM" -H "$AUTH" -H "$UPLOAD_TYPE" -X POST -F file=@${Restore_File} ${HTTP_PREF}/plcm/mcu/api/1.0/system/restore/`basename ${Restore_File}` > "$OUTP" --dump-header "$OUTHEADER" 2> /dev/null

# Test if the MCU service is up. Curl will return an empty file if it cannnot connect.
if [ ! $? -eq 0 ]; then
	echo "There was a problem connecting to the MCU service. Failed restore $(<$OUTP)"
	#exit 3
fi

rm -f $OUTP
rm -f $OUTHEADER

#echo "Get Status"
curl -1 -s -k -H "$PARM" -H "$TYPE" -H "$AUTH" -G ${HTTP_PREF}/plcm/mcu/api/1.0/system/info/state > "$OUTP" --dump-header "$OUTHEADER" 2> /dev/null

# Test if the MCU service is up. Curl will return an empty file if it cannnot connect.
if [ ! $? -eq 0 ]; then
	
	echo "There was a problem connecting to the MCU service. Failed get state $(<$OUTP)"
	#exit 3
fi

rm -f $OUTP
rm -f $OUTHEADER

echo -n "Please wait while restore operation is in progress (dot per 2 seconds) "	

last_size=0
size=1
while [[ $size > $last_size ]]; do
	last_size=$size
	sleep 2
	# getting the restore operation status
	size=`ls -1 $MCU_HOME_DIR/mcms/Restore/Backup*.bck 2> /dev/null | tail -n -1 | tr -s ' ' | cut -d' ' -f5`
	if [[ $size > $last_size ]]; then	
		echo -n "."
	fi
done
echo ""

if [[ ! -f ${Restore_Original_Location}/`basename ${Restore_File}` ]]; then

curl -1 -s -k -H "$PARM" -H "$AUTH" ${HTTP_PREF}/plcm/mcu/api/1.0/system/restore/finish/`basename ${Restore_File}` > "$OUTP"
	echo "Error while copying the backup file $Restore_File"
	rm -f ${Restore_Original_Location}/Backup*.bck
	rm -f $OUTP
	exit 6
fi

# Finialize current restore operation

curl -1 -s -k -H "$PARM" -H "$AUTH" -X POST ${HTTP_PREF}/plcm/mcu/api/1.0/system/restore/finish/`basename ${Restore_File}` > "$OUTP" --dump-header "$OUTHEADER" 2> /dev/null

if [ $? -eq 0 ]; then
        idResult=`cat "$OUTP" |  grep -o -P '(?<=<ID>).*(?=</ID>)'`
        if [[ "$idResult" == "0" ]] || [[ "$idResult" == "" ]]; then
                echo "Restore operation completed successfully"
                echo "Please restart the soft_mcu to activate the restore"      
        else
                echo "Restore operation failed"
                exit 4
        fi
else
        echo "Restore operation failed"
        exit 4
fi

rm -f $OUTP
rm -f $OUTHEADER

exit 0

