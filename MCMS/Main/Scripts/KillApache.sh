#!/bin/sh


#       ALRM	 14   exit
#       HUP	  1   exit
#       INT	  2   exit
#       KILL	  9   exit	this signal may not be blocked
#       PIPE	 13   exit
#       POLL	      exit
#       PROF	      exit
#       TERM	 15   exit
#       USR1	      exit
#       USR2	      exit
#       VTALRM	      exit
#       STKFLT	      exit	may not be implemented
#       PWR	      ignore	may exit on some systems
#       WINCH	      ignore
#       CHLD	      ignore
#       URG	      ignore
#       TSTP	      stop	may interact with the shell
#       TTIN	      stop	may interact with the shell
#       TTOU	      stop	may interact with the shell
#       STOP	      stop	this signal may not be blocked
#       CONT	      restart	continue if stopped, otherwise ignore
#       ABRT	  6   core
#       FPE	  8   core
#       ILL	  4   core
#       QUIT	  3   core
#       SEGV	 11   core
#       TRAP	  5   core
#       SYS	      core	may not be implemented
#       EMT	      core	may not be implemented

export LD_LIBRARY_PATH=$MCU_HOME_DIR/mcms/Bin 
export SASL_PATH=$MCU_HOME_DIR/mcms/Bin


# kill the MCMS module only, it does not inflience the https.
Bin/McuCmd kill ApacheModule

sleep 1

# kill the httpd father process
#kill -TERM `cat $MCU_HOME_DIR/tmp/httpd.pid`
if [ `whoami` == 'root' ]; then
	killall -KILL httpd
else
	sudo pkill httpd
fi

if [ `whoami` == 'root' ]; then
	killall -KILL httpd
else
	sudo killall -q -9 httpd lt-httpd
fi

killall ApacheModule

#sleep 1

