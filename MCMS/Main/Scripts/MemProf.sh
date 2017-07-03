#!/bin/sh
# Memtest.sh

if [ "$1" = "" ]
  then
    all=$(ls $MCU_HOME_DIR/mcms/Bin/* | grep -v "ApacheModule" | grep -v "cs" | grep -v "lib")
  else
    all=$*
fi

rm -Rf $MCU_HOME_DIR/mcms/Links/*
rm -Rf $MCU_HOME_DIR/output/tmp/*mem.*

for process in $all
do
  process=$(basename $process)
  if test -e $MCU_HOME_DIR/mcms/Bin/$process
    then
      echo "#!/bin/sh" >> $MCU_HOME_DIR/mcms/Links/$process
      echo -n "valgrind --tool=callgrind " >> $MCU_HOME_DIR/mcms/Links/$process
      echo -n "--trace-children=no --num-callers=9 " >> $MCU_HOME_DIR/mcms/Links/$process
      echo -n "--callgrind-out-file=$MCU_HOME_DIR/output/tmp/"$process".callgrind " >> $MCU_HOME_DIR/mcms/Links/$process
      echo -n "--log-file=$MCU_HOME_DIR/output/tmp/"$process".prof " >> $MCU_HOME_DIR/mcms/Links/$process

      if [ $process = "ApacheModule" ]
        then
          echo /usr/sbin/httpd -f $MCU_HOME_DIR/mcms/StaticCfg/httpd.conf >> $MCU_HOME_DIR/mcms/Links/$process
        else
          echo $MCU_HOME_DIR/mcms/Bin/$process >> $MCU_HOME_DIR/mcms/Links/$process
      fi

      chmod a+x $MCU_HOME_DIR/mcms/Links/$process
      echo $process "will run next time under profiler."
    else
      echo "No such process - "$process
  fi
done

echo "Please restart this system in order to test memory errors and leaks."
echo "When you finish run MemTestStop.sh instead of reseting the system."
