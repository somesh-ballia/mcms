#!/bin/bash
###################################
# MCU_Backup.sh
#
# Written by Shachar Bar
# <shachar.bar@polycom.co.il>
###################################


Help() 
{

	if [[ "$PRODUCT_TYPE" == "SOFT_MCU_MFW" ]] ; then
		echo "Usage: $0 destination_directory -u <User Name> -p <User Password>"
	else
		echo "Usage: $0 destination_directory [ -u <User Name> -p <User Password> ] "
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

Simulation="NO"
if [ $# -gt 1 ]; then
	Simulation=$2
	if [[ $Simulation == "YES" ]];then
		echo "Working on simulaiton"
	fi
fi

MCU_HOME_DIR=
if [[ $Simulation!="YES" ]];then
	MCU_HOME_DIR=$(grep MCU_HOME_DIR /home/mcms/.bashrc | cut -d'=' -f2 | awk 'NR==1')
fi
export MCU_LIBS=$MCU_HOME_DIR/lib:$MCU_HOME_DIR/lib64:$MCU_HOME_DIR/usr/lib:$MCU_HOME_DIR/usr/lib64

# Variable definitions:
OUTP="$MCU_HOME_DIR/tmp/result.out"
TYPE="Content-Type: application/vnd.plcm.dummy+xml"
PARM="If-None-Match: -1"
SOCKET=`grep '^Listen' $MCU_HOME_DIR/tmp/httpd.listen.conf | head -1 | cut -d' ' -f2`
IP=`echo -n $SOCKET | cut -d':' -f1`
PORT=`echo -n $SOCKET | cut -d':' -f2`
Backup_Original_Location="$MCU_HOME_DIR/mcms/Backup"
IS_SECURE=`echo -n \`cat $MCU_HOME_DIR/mcms/Cfg/NetworkCfg_Management.xml | grep '<IS_SECURED>true</IS_SECURED>'\``



if [[ "$IS_SECURE" != ""  &&  "$PORT" == "443" ]]; then
        HTTP_PREF="https://$IP"
elif [ "$IS_SECURE" != "" ]; then
        HTTP_PREF="https://$IP:$PORT"
else
        HTTP_PREF="http://$IP:$PORT"
fi


echo "Protocol: $HTTP_PREF"

##################################
# Interrupt Handler
##################################
Interrupt() {
	echo "Backup operation was interrupted by the user"
	curl -1 -s -k -H "$PARM" -H "$TYPE" -H "$AUTH" -X POST ${HTTP_PREF}/plcm/mcu/api/1.0/system/backup/finish > "$OUTP"
	rm -f $OUTP
	exit 5
}
##################################
# Interrupt during backup operaiton
trap Interrupt INT TERM


# Checking for the directory parameter
if [[ "X$1" == "X" ]]; then
	echo "Missing destination directory"
	Help	
fi

# Checking for valid destination directory
if [[ ! -d $1 ]]; then
	echo "Destination directorey $1 doesn't exist"
	Help	
fi

# Making sure /mcu_custom_config is with mcms ownership
if [[ `whoami` == 'root' ]]; then
	chown -R mcms.mcms /mcu_custom_config
fi
if [[ $? > 0 ]]; then
   echo "Preparing /mcu_custom_config to be restored failed"
   exit 7
fi



# Staring the backup operation
curl -1 -s -k -H "$PARM" -H "$TYPE" -H "$AUTH" -X POST ${HTTP_PREF}/plcm/mcu/api/1.0/system/backup/start > "$OUTP"

# Checking backup status

# Test if the MCU service is up. Curl will return an empty file if it cannnot connect.
if [ ! $? -eq 0 ]; then
	echo "There was a problem connecting to the MCU service."
	exit 3
fi


if grep -q "Invalid Login" "$OUTP"; then
	echo "Can't start backup operation - error in login"
	exit 3
fi

if grep -q "Cannot encode unknown character set" "$OUTP"; then
	echo "Can't start backup operation - Cannot encode unknown character set"
	exit 3
fi

if grep -q "Invalid Login" "$OUTP"; then
	echo "Can't start backup operation - error in login"
	exit 3
fi

if grep -q "Backup in progress" "$OUTP"; then
	echo "There is another operation currently in progress" 
	exit 3
fi


if grep -q "No credentials" "$OUTP"; then
	echo "No credentials: $user password $password AUTH $AUTH" 
	exit 3
fi

echo -n "Please wait while backup operation is in progress (dot per 2 seconds) "

# Get backup file name
Backup_File_XML=`curl -1 -s -k -H "$PARM" -H "$TYPE" -H "$AUTH" -G ${HTTP_PREF}/plcm/mcu/api/1.0/system/directory/Backup`
Backup_File=`echo $Backup_File_XML | sed -e 's#<[^>]*>##g'`


# Check that backup operation is completed
last_size=0
size=1
while [[ $size > $last_size ]]; do
	sleep 2
	last_size=$size
	size=`ls -l $Backup_Original_Location/$Backup_File | tr -s ' ' | cut -d' ' -f5`
	if [[ $size > $last_size ]]; then
		Prev_Size=$Size
		echo -n "."
	fi
done
echo ""

# Copy the Backup file to the required destination directory
cp -f $Backup_Original_Location/$Backup_File $1

if [[ $? > 0 ]]; then
	echo "Backup file couldn't be created in the destination directory $1"
	exit 4
fi 

# Finialize current backup operation
curl -1 -s -k -H "$PARM" -H "$TYPE" -H "$AUTH" -X POST ${HTTP_PREF}/plcm/mcu/api/1.0/system/backup/finish > "$OUTP"

echo "Backup file is located in: $1/$Backup_File"

rm -f $OUTP
exit 0

