#!/bin/sh

# returns 0 for not self sign certificate
# returns 1 for self sign certificate
# returns 2 for file does not exist

Is_Self_Certificate()
{
	if [ ! -f $1 ]
	then
	    echo "Certificate not found";
            return 2;
	fi
	# parse from certificate the issue to and issue by
   	ISSUED_TO=`$OPENSSL_EXE x509 -text -noout -in  $1 | grep Subject: | grep -o -e "CN=.*$" | sed 's/CN=//'`
    	echo Issued To:$ISSUED_TO
    	ISSUED_BY=`$OPENSSL_EXE x509 -text -noout -in  $1 | grep Issuer: | grep -o -e "CN=.*$" | sed 's/CN=//'`
    	echo Issued By:$ISSUED_BY

    	if [ $ISSUED_TO == $ISSUED_BY ]
    	then
		echo "Certificate is self signed";
		return 1;
	else
		echo "Certificate is signed by ca";
		return 0;
	fi
}

Is_Password_Match_Key()
{

	if [ -f $PRIVATE3_KEY_DIR ]
	then
		echo "check passpharse validation  " 
		$OPENSSL_EXE rsa -in $PRIVATE3_KEY_DIR -out /dev/null -passin pass:$PASS  
		#$OPENSSL_EXE rsa -in $PRIVATE3_KEY_DIR -out /dev/null -passin pass:1234
		isPassMatch=$?;
					
		if [ $isPassMatch -eq 0 ]			
		then
		   echo $isPassMatch
			echo " match key  OK";
			return 1;
		else
			echo "password did not match key create indication file";				
			touch $MNGMT_KEYS_DIR$VALID_IND
			return 0;
		fi 
		 
	fi
	
	return 1;
}



Check_certificate_on_boot () {

	Create_managment_certificate_on_boot       

	Create_ip_services_certificate_on_boot
}

Create_managment_certificate_on_boot () {

	echo "Create_managment_certificate_on_boot"
        #check if self sign certificates
	Is_Self_Certificate $PUBLIK_KEY_DIR
	isSelfSign=$?;
	echo "isSelfSign = " $isSelfSign;	
	
	# if ca certificate then return 
	if [ "$isSelfSign" == 0 ]
	then
		return;
	fi
	
	Is_Password_Match_Key
	
	FAIL_VALID_FILE=$MNGMT_KEYS_DIR$VALID_IND	

	if [ ! -f $PUBLIK_KEY_DIR ]
	then
    		echo "Certificate not found--create new one";
    		Create_certificate management $CN $MNGMT_KEYS_DIR 
	else
		echo $SUFFIX
    		ISSUED_TO=`$OPENSSL_EXE x509 -text -noout -in  $PUBLIK_KEY_DIR | grep Subject: | grep -o -e "CN=.*$" | sed 's/CN=//'`
    		echo Issued To:$ISSUED_TO    		
		
		#check for fail validty indication 
		if [ -f $FAIL_VALID_FILE ] 
		then
			echo "Self signed certificate is not valid create a new one"
			rm $FAIL_VALID_FILE
			Create_certificate management $CN $MNGMT_KEYS_DIR
		else
			#check for change in common name
			if  [ "$ISSUED_TO" != "$CN" ] 
			then
				echo "Self signed certificate host name is different than mcu configuration-need to create a new one"
				Create_certificate management $CN $MNGMT_KEYS_DIR
			fi 			
			if [[ ! -e ${PRIVATE3_KEY_DIR} ]]
			then
				Create_certificate management $CN $MNGMT_KEYS_DIR
			else
				SIZE=$(ls -l ${PRIVATE3_KEY_DIR} | awk -F ' ' '{print $5}')
				if [[ "0" == "${SIZE}" ]]
				then
					Create_certificate management $CN $MNGMT_KEYS_DIR
				fi
			fi

		fi
    					
    		
	fi
	

}


Create_ip_services_certificate_on_boot () {
		
	echo "Create_ip_services_certificate_on_boot"	
	NUMBER_OF_IP_SERVICES=`Get_Number_of_ip_serives`
	echo "NUMBER_OF_IP_SERVICES" $NUMBER_OF_IP_SERVICES
 	for i in `seq 1 $NUMBER_OF_IP_SERVICES`;
        do
		echo " "
		echo "IP Service number $i"
		FAIL_VALID_FILE=$CS_KEYS_DIR/cs$i/$VALID_IND;
		if [ ! -f $CS_KEYS_DIR/cs$i/cert.pem ]
		then	
			echo "Certificate for service $CS_KEYS_DIR/cs$i does not exist ---> creating self sign certificate"	
			if [ -f $FAIL_VALID_FILE ] 
		    	then
				echo "New service create certificate with hostname from file"
				hostname=$(head -1 $FAIL_VALID_FILE)
				rm $FAIL_VALID_FILE
				Create_certificate cs $hostname $CS_KEYS_DIR/cs$i
		    	else
				echo "create certificate with default hostname"
	                	Create_certificate cs PolycomMCU $CS_KEYS_DIR/cs$i
			fi
		else
		    echo "check if self sign certificates"	
	  	    Is_Self_Certificate $CS_KEYS_DIR/cs$i/cert.pem
  	            isSelfSign=$?;
	            echo "isSelfSign = " $isSelfSign;

		    echo "if ca certificate then return" 
	   	    if [ "$isSelfSign" == 0 ]
		    then
			 echo "Certificate is signed by ca";
			 return;
		    fi	
		    echo "check for fail validty indication" 		    	
		    if [ -f $FAIL_VALID_FILE ] 
		    then
			echo "Self signed certificate is not valid create a new one"
			hostname=$(head -1 $FAIL_VALID_FILE)
			rm $FAIL_VALID_FILE
			Create_certificate cs $hostname $CS_KEYS_DIR/cs$i
		    fi
		fi
        done    
	
	

}

Get_Number_of_ip_serives () {

ID_LIST=""
if [ -f $MCU_HOME_DIR/mcms/Cfg/IPMultipleServicesTwoSpansList.xml ];then
		NUMBER_OF_IP_SERVICES=`grep "<IP_SERVICE>" $MCU_HOME_DIR/mcms/Cfg/IPMultipleServicesTwoSpansListTmp.xml | wc -l`
		ID_LIST=`grep '\<SERVICE_ID' $MCU_HOME_DIR/mcms/Cfg/IPMultipleServicesTwoSpansListTmp.xml | cut -d'>' -f2 | cut -d'<' -f1`
	else
		NUMBER_OF_IP_SERVICES=`grep "<IP_SERVICE>" $MCU_HOME_DIR/mcms/Cfg/IPServiceListTmp.xml | wc -l`
		ID_LIST=`grep '\<SERVICE_ID' $MCU_HOME_DIR/mcms/Cfg/IPServiceListTmp.xml | cut -d'>' -f2 | cut -d'<' -f1`
	fi

	
	
	for ID in $ID_LIST
        do
                if [[ $ID -gt   $NUMBER_OF_IP_SERVICES ]];
                then
                        NUMBER_OF_IP_SERVICES=$ID
                fi
        done
	
 	echo $NUMBER_OF_IP_SERVICES
}

Create_certificate () {	
	if [[ $1 == "management" ]];then

		
		rm $PRIVATE3_KEY_DIR;

		KEY=$3/private.pem
		CRT=$3/cert_off.pem
	elif [[ $1 == "cs" ]];then
		KEY=$3/pkey.pem
		CRT=$3/cert.pem
	fi
        CSR=$3/server.csr
	
	#PASS="`hostname`_`date +'%d.%m.%Y.%H%M%S'`"
	
	O="Polycom RMX"
	OU="Self Signed Certificate"
	DC="polycom"	
	CN=$2

        echo CN:$CN

        # Create a key and a self signed certificate
	
	if [[ "$SOFT_MCU_FAMILY" == "" || "$SOFT_MCU_FAMILY" != "YES" ]];then
		$OPENSSL_EXE genrsa -des3 -passout pass:$PASS -out $KEY 2048	
	else
		$OPENSSL_EXE genrsa -passout pass:$PASS -out $KEY 2048	
	fi 
        #$OPENSSL_EXE genrsa $DES -passout pass:$PASS -out $KEY 2048
	#$OPENSSL_EXE genrsa -passout pass:$PASS -out $KEY 2048
	echo $IS_TARGET
	
	

        $OPENSSL_EXE req -new -key $KEY -passin pass:$PASS -out $CSR -subj "/DC=$DC/O=$O/OU=$OU/CN=$CN"	
        $OPENSSL_EXE x509 -req -days 3650 -in $CSR -signkey $KEY -out $CRT -passin pass:$PASS        

        # Make a password-less key
        $OPENSSL_EXE rsa -in $KEY -out $KEY.insec -passin pass:$PASS

        $OPENSSL_EXE x509 -text -noout -in  $CRT
        mv $KEY $KEY.sec
        mv $KEY.insec $KEY
	if id -u mcms >/dev/null 2>&1; then
		echo "user mcms exists. change owner of certificate files to mcms"
		chmod 640 $KEY
		chown mcms:mcms $KEY		
		chown mcms:mcms $CRT		
	fi    
	
	echo "delete temp files"
	if [  -f $CSR ]
	then
	    rm $CSR
	fi

	rm $KEY.sec
}

Get_serial_number () {
	
	$OPENSSL_EXE x509 -text -noout -in $1  > cert_output
	Serial_Number=`sed -n '/Serial Number:/,/Signature Algorithm:/p' cert_output | awk NR==2  | sed 's/://g'  | tr '[a-z]' '[A-Z]'  | sed 's/ //g'`	
	echo $Serial_Number

}
#################################################################
#								#
#			MAIN					#
#								#
#################################################################   

export RANDFILE=$MCU_HOME_DIR/tmp
PRODUCT_TYPE=$(cat $MCU_HOME_DIR/mcms/ProductType)
PRIVATE3_KEY_DIR="$MCU_HOME_DIR/mcms/Keys/private3.pem"
PUBLIK_KEY_DIR="$MCU_HOME_DIR/mcms/Keys/cert_off.pem"
MNGMT_KEYS_DIR="$MCU_HOME_DIR/mcms/Keys/"
CS_KEYS_DIR="$MCU_HOME_DIR/mcms/KeysForCS"
VALID_IND="cert_valid_fail.ind"
PASS=`$MCU_HOME_DIR/mcms/Scripts/TLS/passphrase.sh`
SSL_OUT_FILE="$MCU_HOME_DIR/mcms/Keys/openssl_out.txt"
OPENSSL_EXE="$MCU_HOME_DIR/mcms/Bin/openssl"

export LD_LIBRARY_PATH=$MCU_HOME_DIR/mcms/Bin/
export SASL_PATH=$MCU_HOME_DIR/mcms/Bin

if [[ "$PRODUCT_TYPE" == "SOFT_MCU_EDGE" || "$PRODUCT_TYPE" == "SOFT_MCU_MFW"  || "$PRODUCT_TYPE" == "SOFT_MCU" ]];then
	CN=`hostname`
else 
	CN=`sed -n 's|<HOST_NAME>\(.*\)</HOST_NAME>|\1|p' $MCU_HOME_DIR/mcms/Cfg/NetworkCfg_Management.xml | head -1`
fi

CN=`echo $CN | sed 's/ //g'`
if [[ "" == "${CN}" ]]
then
	CN="PolycomMCU"
fi

echo "CN:" $CN
export OPENSSL_CONF="$MCU_HOME_DIR/mcms/StaticCfg/openssl.cnf"
echo "OpenSSL Configuration File $OPENSSL_CONF"


if [[ $1 == "boot" ]];
then
	Check_certificate_on_boot
fi

if [[ $1 == "Create_managment_certificate" ]];
then
	touch $MNGMT_KEYS_DIR$VALID_IND
fi

if [[ $1 == "Create_ip_service_certificate" ]];
then	
	#create indication file with hostname
        
	touch $CS_KEYS_DIR/cs$3/$VALID_IND ;	
	echo "$2" > $CS_KEYS_DIR/cs$3/$VALID_IND ;	
	echo "create indication file with hostname " $CS_KEYS_DIR/cs$3/$VALID_IND " " $2
fi

if [[ $1 == "Delete_ip_service_certificate" ]];
then	
	rm $CS_KEYS_DIR/cs$2/*
fi

if [[ $1 == "Recreate_certificate" ]];
then	
	Serial_Number=`Get_serial_number $PUBLIK_KEY_DIR`
	echo "mngmt serial" $Serial_Number
	echo $2
	if [[ "$2" == "$Serial_Number" ]];then
		echo "mngmt cert"
		Create_certificate management $3 $MNGMT_KEYS_DIR
		if [ -f "$PRIVATE3_KEY_DIR" ];then
			rm $PRIVATE3_KEY_DIR
		fi
	else
		NUMBER_OF_IP_SERVICES=`Get_Number_of_ip_serives`
		echo "NUMBER_OF_IP_SERVICES" $NUMBER_OF_IP_SERVICES
	 	for i in `seq 1 $NUMBER_OF_IP_SERVICES`;
	        do
			if [ -f $CS_KEYS_DIR/cs$i/cert.pem ];then
				Serial_Number=`Get_serial_number $CS_KEYS_DIR/cs$i/cert.pem`
				echo "cs serial" $Serial_Number
				if [[ "$2" == "$Serial_Number" ]];then
					echo "cs cert"
					Create_certificate cs $3 $CS_KEYS_DIR/cs$i
		
				fi
	                	
			fi
	        done	

	fi

fi
