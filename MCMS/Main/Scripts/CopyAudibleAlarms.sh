#!/bin/sh

#in case the user has changed the files - don't override them with the files from the new version  

# create empty folders
cd $MCU_HOME_DIR/mcms/StaticCfg/
find AudibleAlarms  -type d > $MCU_HOME_DIR/tmp/audio.tmp
cd $MCU_HOME_DIR/mcms/Cfg/
mkdir -p `cat $MCU_HOME_DIR/tmp/audio.tmp`
rm $MCU_HOME_DIR/tmp/audio.tmp
chmod -R a+w $MCU_HOME_DIR/mcms/Cfg/AudibleAlarms
chown -R mcms:mcms $MCU_HOME_DIR/mcms/Cfg/AudibleAlarms

cd $MCU_HOME_DIR/mcms/StaticCfg/
find AudibleAlarms  -type f | while read file;
do
  echo $file
  if test -e $MCU_HOME_DIR/mcms/Cfg/$file; then
    #need to verify
    md5sum $MCU_HOME_DIR/mcms/Cfg/$file > $MCU_HOME_DIR/tmp/md5sum.tmp
    cat $MCU_HOME_DIR/tmp/md5sum.tmp 
    cat $MCU_HOME_DIR/mcms/Cfg/$file.md5sum
    if diff $MCU_HOME_DIR/tmp/md5sum.tmp $MCU_HOME_DIR/mcms/Cfg/$file.md5sum; then
       echo "md5 is the same - copy updated file from version"
       echo "Copying $MCU_HOME_DIR/mcms/StaticCfg/$file"
       cp $MCU_HOME_DIR/mcms/StaticCfg/$file $MCU_HOME_DIR/mcms/Cfg/$file
       md5sum $MCU_HOME_DIR/mcms/Cfg/$file > $MCU_HOME_DIR/mcms/Cfg/$file.md5sum    
    else    
       echo "md5 is not the same!!!, leaving the file as is"
    fi
    rm $MCU_HOME_DIR/tmp/md5sum.tmp
  else
    echo "missing: Copying $MCU_HOME_DIR/mcms/StaticCfg/$file"
    cp $MCU_HOME_DIR/mcms/StaticCfg/$file $MCU_HOME_DIR/mcms/Cfg/$file
    md5sum $MCU_HOME_DIR/mcms/Cfg/$file > $MCU_HOME_DIR/mcms/Cfg/$file.md5sum
  fi
done
