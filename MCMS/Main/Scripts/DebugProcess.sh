#!/bin/sh
#parameters : 
# $1 - debug
# $2 - process name
# $3 - command for gdb

echo the $1 process will run under gdb
echo $2 > run.txt
echo y >> run.txt
echo run >> run.txt
echo "#!/bin/sh" > Links/$1
echo "xterm -e gdb --command=run.txt Bin/$1" >> Links/$1

 
  
