#!/bin/sh
dan=`cat /proc/meminfo | head -4 | awk '{print $2}'`

# we use constant as 100 percent, even if we have 2 * 513912 
#echo $dan | awk '{print 100*(($2+$4)/513912)}'

#kobi g , accroding to what we see (Me and oded) the calculation is in correct , we revert back to original and will give an error below 10%
# this is the original formulae. the 100 percent is dymamic.
echo $dan | awk '{print 100*(($2+$4)/$1)}'
