#!/bin/sh
# MemTest.sh

if [ "$1" = "--help" ]
then
	echo "   Run single process under  valgrind: .\MemTest.sh ConfParty"
	echo "   Run few  processes under  valgrind: .\MemTest.sh ConfParty Resource CDR"
	echo "   Run all  processes under  valgrind: .\MemTest.sh"
	echo "   Run single process under callgrind: .\MemTest.sh ConfParty prof"
	echo "   Run few  processes under callgrind: .\MemTest.sh ConfParty prof Resource CDR"
	exit
fi

if [ "$1" = "" ]
  then
    all=$(ls $MCU_HOME_DIR/mcms/Bin/* | grep -v "ApacheModule" | grep -v "cs" | grep -v "lib")
  else
    all=$*
fi

rm -Rf $MCU_HOME_DIR/mcms/Links/*
rm -Rf $MCU_HOME_DIR/output/tmp/*mem.*
rm -Rf $MCU_HOME_DIR/output/tmp/*.prof
rm -Rf $MCU_HOME_DIR/output/tmp/*.callgrind.out.*

for process in $all
do
  process=$(basename $process)
  if test -e $MCU_HOME_DIR/mcms/Bin/$process
    then
      if [ "$2" = "prof" ]
      then
        echo "#!/bin/sh" >> $MCU_HOME_DIR/mcms/Links/$process
        echo -n "valgrind --tool=callgrind " >> $MCU_HOME_DIR/mcms/Links/$process
        echo -n "--trace-children=no --num-callers=8 " >> $MCU_HOME_DIR/mcms/Links/$process
        echo -n "--log-file=$MCU_HOME_DIR/output/tmp/"$process".prof " >> $MCU_HOME_DIR/mcms/Links/$process
        echo -n "--callgrind-out-file=$MCU_HOME_DIR/output/tmp/$process.callgrind.out.%p " >> $MCU_HOME_DIR/mcms/Links/$process
        echo $MCU_HOME_DIR/mcms/Bin/$process >> $MCU_HOME_DIR/mcms/Links/$process
        echo $process "will run next time in callgrind profiler"
      else
        echo "#!/bin/sh" >> $MCU_HOME_DIR/mcms/Links/$process
        echo -n "valgrind --tool=memcheck --leak-check=yes " >> $MCU_HOME_DIR/mcms/Links/$process
        echo -n "--trace-children=no --num-callers=9 " >> $MCU_HOME_DIR/mcms/Links/$process
        echo -n "--error-limit=no --show-reachable=yes " >> $MCU_HOME_DIR/mcms/Links/$process
        echo -n "--log-file=$MCU_HOME_DIR/output/tmp/"$process".mem " >> $MCU_HOME_DIR/mcms/Links/$process

        if [ $process = "ApacheModule" ]
          then
            echo /usr/sbin/httpd -f $MCU_HOME_DIR/mcms/StaticCfg/httpd.conf >> $MCU_HOME_DIR/mcms/Links/$process
          else
            echo $MCU_HOME_DIR/mcms/Bin/$process >> $MCU_HOME_DIR/mcms/Links/$process
        fi
        echo $process "will run next time under valgrind."
      fi         

      chmod a+x $MCU_HOME_DIR/mcms/Links/$process
  else
      echo "No such process - "$process
  fi
done

echo "Please restart this system in order to test memory errors and leaks."
echo "When you finish run MemTestStop.sh instead of reseting the system."