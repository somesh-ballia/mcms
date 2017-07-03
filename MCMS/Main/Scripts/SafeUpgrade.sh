#!/bin/sh

SAFE_UPGRADE_LOG=$MCU_HOME_DIR/tmp/startup_logs/safeUpgrade.log	




echo  "I am in safe upgrade script" >> $SAFE_UPGRADE_LOG



txt_file=$1

version_to_check=$2

current_version=$3

new_version=$4

product_type=$5

card_type=$6

echo "txt_file "  $1 >> $SAFE_UPGRADE_LOG
echo "version_to_check" $2 >> $SAFE_UPGRADE_LOG
echo "current_version" $3 >> $SAFE_UPGRADE_LOG
echo "new_version" $4 >> $SAFE_UPGRADE_LOG
echo "product_type" $5 >> $SAFE_UPGRADE_LOG
echo "card_type" $6 >> $SAFE_UPGRADE_LOG


BLACK_LIST="ALL_PRODUCTS_ALL_CARDS_BLACK_LIST"
WHITE_LIST="ALL_PRODUCTS_ALL_CARDS_WHITE_LIST"

BLACK_LIST_ALL=`cat $txt_file | grep $BLACK_LIST |awk -F ':' '{print $2}'`
WHITE_LIST_ALL=`cat $txt_file | grep $WHITE_LIST |awk -F ':' '{print $2}'`

echo "BLACK_LIST_ALL" $BLACK_LIST_ALL >> $SAFE_UPGRADE_LOG
echo "WHITE_LIST_ALL" $WHITE_LIST_ALL >> $SAFE_UPGRADE_LOG

BLACK_TAG=$product_type"_"$card_type"_BLACK_LIST"
WHITE_TAG=$product_type"_"$card_type"_WHITE_LIST"

echo "BLACK_TAG" $BLACK_TAG  >> $SAFE_UPGRADE_LOG
echo "WHITE_TAG" $WHITE_TAG  >> $SAFE_UPGRADE_LOG

BLACK_LIST=`cat $txt_file | grep $BLACK_TAG |awk -F ':' '{print $2}'`
WHITE_LIST=`cat $txt_file | grep $WHITE_TAG |awk -F ':' '{print $2}'`

echo "BLACK_LIST" $BLACK_LIST >> $SAFE_UPGRADE_LOG
echo "WHITE_LIST" $WHITE_LIST  >> $SAFE_UPGRADE_LOG

#************************************************************************************************************************
#
#  check the black list in all products and all cards
#
#************************************************************************************************************************

echo "check the BLACK LIST in ALL PRODUCTS and ALL CARDS" >> $SAFE_UPGRADE_LOG

index=1
version=`echo $BLACK_LIST_ALL | awk -F ';' '{print $'$index'}'`
while [ "$version" != "" ];
      do 
         echo  $version >> $SAFE_UPGRADE_LOG

         
         if [ "$version" == "ALL" ];
          then
           echo "found ALL in all products all cards black list" >> $SAFE_UPGRADE_LOG
           exit 1
          fi
          
         if [ "$version" == "$version_to_check" ];
          then
           echo "found version match in all products all cards black list" $version >> $SAFE_UPGRADE_LOG
           exit 1
          fi
          
          index=`expr $index + 1`
          version=`echo $BLACK_LIST_ALL | awk -F ';' '{print $'$index'}'`
    
         
         
         
     done 
     
     
#************************************************************************************************************************
#
#  check the black list in current product type and card type
#
#************************************************************************************************************************

echo "check the BLACK LIST in CURRENT PRODUCT and CURRENT CARD" >> $SAFE_UPGRADE_LOG

index=1
version=`echo $BLACK_LIST | awk -F ';' '{print $'$index'}'`
while [ "$version" != "" ];
      do 
         echo  $version >> $SAFE_UPGRADE_LOG

         
         if [ "$version" == "ALL" ];
          then
           echo "found ALL in current product and card black list" >> $SAFE_UPGRADE_LOG
           exit 1
          fi
          
         if [ "$version" == "$version_to_check" ];
          then
           echo "found version match in current product and card black list" $version >> $SAFE_UPGRADE_LOG
           exit 1
          fi
          
          index=`expr $index + 1`
          version=`echo $BLACK_LIST | awk -F ';' '{print $'$index'}'`        
         
     done 
     


#************************************************************************************************************************
#
#  check the white list in all products and all cards
#
#************************************************************************************************************************

echo "check the WHITE LIST in ALL PRODUCTS and ALL CARDS" >> $SAFE_UPGRADE_LOG
     
index=1
version=`echo $WHITE_LIST_ALL | awk -F ';' '{print $'$index'}'`
while [ "$version" != "" ];
      do 
         echo  $version >> $SAFE_UPGRADE_LOG

         
         if [ "$version" == "ALL" ];
          then
           echo "found ALL in all products all cards white list" >> $SAFE_UPGRADE_LOG
           exit 2
          fi
          
         if [ "$version" == "$version_to_check" ];
          then
           echo "found version match in all products all cards white list" $version >> $SAFE_UPGRADE_LOG
           exit 2
          fi
          
          index=`expr $index + 1`
          version=`echo $WHITE_LIST_ALL | awk -F ';' '{print $'$index'}'`        
         
     done 
     
#************************************************************************************************************************
#
#  check the white list in current product type and card type
#
#************************************************************************************************************************

echo "check the WHITE LIST in CURRENT PRODUCT and CURRENT CARD" >> $SAFE_UPGRADE_LOG
     
index=1
version=`echo $WHITE_LIST | awk -F ';' '{print $'$index'}'`
while [ "$version" != "" ];
      do 
         echo  $version >> $SAFE_UPGRADE_LOG

         
         if [ "$version" == "ALL" ];
          then
           echo "found ALL in current product and card black list" >> $SAFE_UPGRADE_LOG
           exit 2
          fi
          
         if [ "$version" == "$version_to_check" ];
          then
           echo "found version match in current product and card white list" $version >> $SAFE_UPGRADE_LOG
           exit 2
          fi
          
          index=`expr $index + 1`
          version=`echo $WHITE_LIST | awk -F ';' '{print $'$index'}'`        
         
     done 
#*********************************************************************************************************
    

cut_version=`echo $version_to_check | awk -F '.' '{print $1"."$2"."$3}'`
echo "cut_version" $cut_version >> $SAFE_UPGRADE_LOG
if [ "$cut_version" != "$version_to_check" ];
then
echo " version to check x1.x2.x3---------------> "$cut_version  >> $SAFE_UPGRADE_LOG
#***********************************************************
#
#  check the black list in all products and all cards
#
#***********************************************************

echo "check the BLACK LIST in ALL PRODUCTS and ALL CARDS" >> $SAFE_UPGRADE_LOG

index=1
version=`echo $BLACK_LIST_ALL | awk -F ';' '{print $'$index'}'`
while [ "$version" != "" ];
      do 
         echo  $version >> $SAFE_UPGRADE_LOG

         
         if [ "$version" == "ALL" ];
          then
           echo "found ALL in all products all cards black list" >> $SAFE_UPGRADE_LOG
           exit 1
          fi
          
         if [ "$version" == "$cut_version" ];
          then
           echo "found version match in all products all cards black list" $version >> $SAFE_UPGRADE_LOG
           exit 1
          fi
          
          index=`expr $index + 1`
          version=`echo $BLACK_LIST_ALL | awk -F ';' '{print $'$index'}'`
    
         
         
         
     done 
     
     
#***************************************************************
#
#  check the black list in current product type and card type
#
#***************************************************************

echo "check the BLACK LIST in CURRENT PRODUCT and CURRENT CARD" >> $SAFE_UPGRADE_LOG

index=1
version=`echo $BLACK_LIST | awk -F ';' '{print $'$index'}'`
while [ "$version" != "" ];
      do 
         echo  $version >> $SAFE_UPGRADE_LOG

         
         if [ "$version" == "ALL" ];
          then
           echo "found ALL in current product and card black list" >> $SAFE_UPGRADE_LOG
           exit 1
          fi
          
         if [ "$version" == "$cut_version" ];
          then
           echo "found version match in current product and card black list" $version >> $SAFE_UPGRADE_LOG
           exit 1
          fi
          
          index=`expr $index + 1`
          version=`echo $BLACK_LIST | awk -F ';' '{print $'$index'}'`        
         
     done 
     


#*******************************************************************
#
#  check the white list in all products and all cards
#
#*******************************************************************

echo "check the WHITE LIST in ALL PRODUCTS and ALL CARDS" >> $SAFE_UPGRADE_LOG
    
index=1
version=`echo $WHITE_LIST_ALL | awk -F ';' '{print $'$index'}'`
while [ "$version" != "" ];
      do 
         echo  $version >> $SAFE_UPGRADE_LOG

         
         if [ "$version" == "ALL" ];
          then
           echo "found ALL in all products all cards white list" >> $SAFE_UPGRADE_LOG
           exit 2
          fi
          
         if [ "$version" == "$cut_version" ];
          then
           echo "found version match in all products all cards white list" $version >> $SAFE_UPGRADE_LOG
           exit 2
          fi
          
          index=`expr $index + 1`
          version=`echo $WHITE_LIST_ALL | awk -F ';' '{print $'$index'}'`        
         
     done 
     
#********************************************************************
#
#  check the white list in current product type and card type
#
#********************************************************************

echo "check the WHITE LIST in CURRENT PRODUCT and CURRENT CARD" >> $SAFE_UPGRADE_LOG

     
index=1
version=`echo $WHITE_LIST | awk -F ';' '{print $'$index'}'`
while [ "$version" != "" ];
      do 
         echo  $version >> $SAFE_UPGRADE_LOG

         
         if [ "$version" == "ALL" ];
          then
           echo "found ALL in current product and card white list" >> $SAFE_UPGRADE_LOG
           exit 2
          fi
          
         if [ "$version" == "$cut_version" ];
          then
           echo "found version match in current product and card white list" $version >> $SAFE_UPGRADE_LOG
           exit 2
          fi
          
          index=`expr $index + 1`
          version=`echo $WHITE_LIST | awk -F ';' '{print $'$index'}'`        
         
     done 
     

fi

#***************************************************************************************************************
cut_version=`echo $version_to_check | awk -F '.' '{print $1"."$2}'`
echo "cut_version" $cut_version >> $SAFE_UPGRADE_LOG
if [ "$cut_version" != "$version_to_check" ];
then
echo " version to check x1.x2---------------> "$cut_version >> $SAFE_UPGRADE_LOG
#*************************************************************
#
#  check the black list in all products and all cards
#
#*************************************************************

echo "check the BLACK LIST in ALL PRODUCTS and ALL CARDS" >> $SAFE_UPGRADE_LOG

index=1
version=`echo $BLACK_LIST_ALL | awk -F ';' '{print $'$index'}'`
while [ "$version" != "" ];
      do 
         echo  $version >> $SAFE_UPGRADE_LOG

         
         if [ "$version" == "ALL" ];
          then
           echo "found ALL in all products all cards black list" >> $SAFE_UPGRADE_LOG
           exit 1
          fi
          
         if [ "$version" == "$cut_version" ];
          then
           echo "found version match in all products all cards black list" $version >> $SAFE_UPGRADE_LOG
           exit 1
          fi
          
          index=`expr $index + 1`
          version=`echo $BLACK_LIST_ALL | awk -F ';' '{print $'$index'}'`
    
         
         
         
     done 
     
     
#*************************************************************
#
#  check the black list in current product type and card type
#
#*************************************************************

echo "check the BLACK LIST in CURRENT PRODUCT and CURRENT CARD" >> $SAFE_UPGRADE_LOG

index=1
version=`echo $BLACK_LIST | awk -F ';' '{print $'$index'}'`
while [ "$version" != "" ];
      do 
         echo  $version >> $SAFE_UPGRADE_LOG

         
         if [ "$version" == "ALL" ];
          then
           echo "found ALL in current product and card black list" >> $SAFE_UPGRADE_LOG
           exit 1
          fi
          
         if [ "$version" == "$cut_version" ];
          then
           echo "found version match in current product and card black list" $version >> $SAFE_UPGRADE_LOG
           exit 1
          fi
          
          index=`expr $index + 1`
          version=`echo $BLACK_LIST | awk -F ';' '{print $'$index'}'`        
         
     done 
     


#*******************************************************
#
#  check the white list in all products and all cards
#
#*******************************************************

echo "check the WHITE LIST in ALL PRODUCTS and ALL CARDS" >> $SAFE_UPGRADE_LOG
     
index=1
version=`echo $WHITE_LIST_ALL | awk -F ';' '{print $'$index'}'`
while [ "$version" != "" ];
      do 
         echo  $version >> $SAFE_UPGRADE_LOG

         
         if [ "$version" == "ALL" ];
          then
           echo "found ALL in all products all cards white list" >> $SAFE_UPGRADE_LOG
           exit 2
          fi
          
         if [ "$version" == "$cut_version" ];
          then
           echo "found version match in all products all cards white list" $version >> $SAFE_UPGRADE_LOG
           exit 2
          fi
          
          index=`expr $index + 1`
          version=`echo $WHITE_LIST_ALL | awk -F ';' '{print $'$index'}'`        
         
     done 
     
#**************************************************************
#
#  check the white list in current product type and card type
#
#**************************************************************

echo "check the WHITE LIST in CURRENT PRODUCT and CURRENT CARD" >> $SAFE_UPGRADE_LOG
     
index=1
version=`echo $WHITE_LIST | awk -F ';' '{print $'$index'}'`
while [ "$version" != "" ];
      do 
         echo  $version >> $SAFE_UPGRADE_LOG

         
         if [ "$version" == "ALL" ];
          then
           echo "found ALL in current product and card white list" >> $SAFE_UPGRADE_LOG
           exit 2
          fi
          
         if [ "$version" == "$cut_version" ];
          then
           echo "found version match in current product and card white list" $version >> $SAFE_UPGRADE_LOG
           exit 2
          fi
          
          index=`expr $index + 1`
          version=`echo $WHITE_LIST | awk -F ';' '{print $'$index'}'`        
         
     done 
     

fi


#*********************************************************************************************************
#   
#  check downgrade
#
#*********************************************************************************************************
echo "new_version" $new_version >> $SAFE_UPGRADE_LOG
echo "current_version" $current_version >> $SAFE_UPGRADE_LOG
newVersion_number=`echo $new_version| awk -F '.' '{print $1}'`
currentVersion=`echo $current_version| awk -F '.' '{print $1}'`
echo "newVersion_number" $newVersion_number >> $SAFE_UPGRADE_LOG
echo "currentVersion" $currentVersion >> $SAFE_UPGRADE_LOG
echo ".$currentVersion.$currentVersion." >> $SAFE_UPGRADE_LOG 
echo ".$newVersion_number.$newVersion_number." >> $SAFE_UPGRADE_LOG 
if [ $newVersion_number -lt $currentVersion ];
then
  echo "It is case of downgrade , allow downgrade" >> $SAFE_UPGRADE_LOG
  exit 2
 else echo "It is not a case of downgrade ($newVersion_number,$currentVersion)" >> $SAFE_UPGRADE_LOG
fi


if [ $newVersion_number == $currentVersion ];
then
  echo "first digit equal lets check the second digit" >> $SAFE_UPGRADE_LOG
  newVersion_number=`echo $new_version| awk -F '.' '{print $2}'`
  currentVersion=`echo $current_version| awk -F '.' '{print $2}'`
  if [ $newVersion_number -lt $currentVersion ];
   then
     echo "It is case of downgrade , allow downgrade" >> $SAFE_UPGRADE_LOG
     exit 2
  fi
  if [ $newVersion_number == $currentVersion ];
  then
     echo "second digit equal lets check the third digit" >> $SAFE_UPGRADE_LOG
     newVersion_number=`echo $new_version| awk -F '.' '{print $3}' `
     currentVersion=`echo $current_version| awk -F '.' '{print $3}' `
     if [ $newVersion_number -lt $currentVersion ];
        then
        echo "It is case of downgrade , allow downgrade" >> $SAFE_UPGRADE_LOG
        exit 2
     fi
     if [ $newVersion_number == $currentVersion ];
        then
          echo "third digit equal lets check the third digit" >> $SAFE_UPGRADE_LOG
          newVersion_number=`echo $new_version| awk -F '.' '{print $4}' `
          currentVersion=`echo $current_version| awk -F '.' '{print $4}' `
          if [ $newVersion_number -lt $currentVersion ];
             then
               echo "It is case of downgrade , allow downgrade" >> $SAFE_UPGRADE_LOG
               exit 2
          fi
     fi  
  fi 
fi  
  
echo "end of script" >> $SAFE_UPGRADE_LOG

exit 3


