#!/bin/sh
#
# name  : auto-test-produce-step.sh
# author: david rabkin
# date  : 20100613
#
# produces xml entries for auto-test build forge project file, looks as
#
#    <step absolute="N" failwait="N" filterId="1" selectorId="" dir="/" broadcast="N" timeout="300" id="7" passwait="N" inline="N" threadable="Y" access="6" active="Y" passnotify="0" description="test001" onfail="C" failnotify="0" envId="0">
#      <command>$AT_TOOLS_PATH/BuildTestEnv.sh $test001 $MCMS_MAIN_PATH $TEST_PATH $RESULT_PATH</command>
#    </step>
#
# using:
#        auto-test-produce-step.sh > step.txt

for ii in {0..299}; do

    # start with specified step position
    let POS=7+ii

    # format test number to 3 symbols at least
    TAG=test`printf "%.3d\n" $[ii+1]`

    STRING1="\
        <step absolute=\"N\" \
        failwait=\"N\" \
        filterId=\"1\" \
        selectorId=\"\" \
        dir=\"/\" \
        broadcast=\"N\" \
        timeout=\"300\" \
        id=\"$POS\" \
        passwait=\"N\" \
        inline=\"N\" \
        threadable=\"Y\" \
        access=\"6\" \
        active=\"Y\" \
        passnotify=\"0\" \
        description=\"$TAG\" \
        onfail=\"C\" \
        failnotify=\"0\" \
        envId=\"0\">"

#    STRING2="\
#        <command>\$AT_TOOLS_PATH/BuildTestEnv.sh \
#        \$$TAG \
#        \$MCMS_MAIN_PATH \
#        \$TEST_PATH \
#        \$RESULT_PATH</command>"

    STRING2="<command>\$AT_STEP_CMD</command>"

    STRING3="</step>"
     
    # put some spaces of xml formatting
    echo "    "$STRING1
    echo "      "$STRING2
    echo "    "$STRING3
done
