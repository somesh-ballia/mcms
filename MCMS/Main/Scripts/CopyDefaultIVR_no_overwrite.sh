#!/bin/sh

# using interactive mode the cp always get no for the question "overwrite file?"
# the yes process provides the no (n) answer.



# sagi patch for VNGR-11657
md5_cfg=`md5sum  $MCU_HOME_DIR/mcms/Cfg/IVR/msg/English/welc/welcmsg/General_Welcome.wav  | awk -F ' ' '{ print $1 }'`

if [ $md5_cfg == b8f289c31bcd31b4f2aa996cec526c51 ]
then
    echo "very old General_Welcome.wav , delete it"
    rm -f $MCU_HOME_DIR/mcms/Cfg/IVR/msg/English/welc/welcmsg/General_Welcome.wav
fi

if [ $md5_cfg == eed3fd749c578ada52206dd893a8f1f9 ]
then
    echo "very General_Welcome.wav , delete it"
    rm -f $MCU_HOME_DIR/mcms/Cfg/IVR/msg/English/welc/welcmsg/General_Welcome.wav
fi

# sagi patch to remove empty files from IVR
find $MCU_HOME_DIR/mcms/Cfg/IVR/ |
(
while read file 
do
  type=`stat -c %F $file`
  if [ "$type" == "directory" ]
  then
      echo > /dev/null
  else
      size=`stat -c %s $file`
      if [ $size == 0 ]
      then
	  echo "removing empty file:" $file
	  rm -f $file
      fi
  fi
done
)

cd $MCU_HOME_DIR/mcms/Cfg/IVR/Fonts
cat $MCU_HOME_DIR/mcms/StaticCfg/IVR/font_hash.md5|
(
while read file 
do
 orig_sum=$(echo $file | awk '{print $1}')
 file_name=$(echo $file | awk '{print $2}')
 file_sum=$(md5sum $file_name| awk '{print $1}')
 
 if [ "$file_sum" != "$orig_sum" ] 
 then
  		rm -f  $file_name
	else
    	echo "Checksum passed"
  fi
done
)

cd -

(yes n | cp -dpRi $MCU_HOME_DIR/mcms/StaticCfg/IVR/* $MCU_HOME_DIR/mcms/Cfg/IVR/)

chown -R mcms:mcms $MCU_HOME_DIR/output/IVR/
chmod -R +w $MCU_HOME_DIR/output/IVR/
chmod -R o-w $MCU_HOME_DIR/output/IVR
chmod -R o-w $MCU_HOME_DIR/config/mcms
chmod -R o-w $MCU_HOME_DIR/mcms/Cfg/IVR/msg/English/*
