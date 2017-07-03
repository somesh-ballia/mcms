#!/bin/bash
# scriptname - DeliveryTestsDefs.sh

. ./Scripts/CommonTestsDefs.sh

#
# List of private functions of the file
#
Make_UnitTest (){
  rm -Rf Cfg
  mkdir -p Cfg

  RsltUnitTest=0
  RsltCoreDump=0  
  for TestBinary in Bin/*.Test ; do 
    echo "$COLORBLUE === Running $TestBinary === $TXTRESET"
    Scripts/CleanSharedResources.sh
    $TestBinary || RsltUnitTest=$?
    CheckCoreDumpsExisting || RsltCoreDump=$?
 
    if [ $RsltUnitTest -ne 0  -o  $RsltCoreDump -ne 0 ] 
    then
      echo $COLORRED"Test $TestBinary failed with err $RsltUnitTest"$TXTRESET
      return $RsltUnitTest
    fi  
  done
}

# RunSmthWithoutChkCoredump  ====================
#Notes: This function can run command with one parameter only
RunSmthWithoutChkCoredump (){
  RetCode=0
  $3 $4 || RetCode=$?

  if [ $RetCode -ne 0 ]
  then
    $1 $RetCode $2
  fi

  param=${2-false}
  if [ $param == "true" ]
  then
    $1 0 $2
  fi
}


#
# List of public functions of the file
#
# Exit_From_MakeTest  ===========================
Exit_From_MakeTest (){
  EndMCMSPythonTests
  ret=$1
  
  if [ $ret -eq 0 ]
  then 
    echo $TXTBOLD$COLORWHITE$BGCOLORGREEN
    echo "*************************************************"
    echo "********** RUNNING TEST(S) SUCCEEDED ************"
    echo "*************************************************"
    echo $TXTRESET
  else
    echo $TXTBOLD$COLORWHITE$BGCOLORRED
    echo "*************************************************"
    echo "********** RUNNING TEST(S) FAILED ($ret) ********"
    echo "*************************************************"
    echo $TXTRESET
  fi
  exit $ret
}


#
# List of functions which run tests themselves
#

#--DTD_MCMSUnitTests
DTD_MCMSUnitTests (){
  cd $MCMS_DIR
  make test_bin
  RunSmthWithoutChkCoredump $1 $2 Make_UnitTest
}

#--DTD_SoftMCUSystemTests
DTD_SoftMCUSystemTests (){
  verify_testing_variables
  ret=$?
  if [[ $ret != 0 ]];then
	$1 $ret $2
  fi

  RunSmth $1 $2 Scripts/SoftMcuTests.sh VERSION
}

#--DTD_MCMSAllVersionTestsBreezeMode
DTD_MCMSAllVersionTestsBreezeMode (){
  RunSomeMCMSPython $1 $2 Scripts/RunTest.sh Scripts/AllVersionTestsBreezeMode.sh
}

#--DTD_MCMSAllVersionTestsBreezeModeCP
DTD_MCMSAllVersionTestsBreezeModeCP (){
  RunSomeMCMSPython $1 $2 Scripts/RunTest.sh Scripts/AllVersionTestsBreezeModeCP.sh 
}

#--DTD_MCMSAllVersionTestsBarakMode
DTD_MCMSAllVersionTestsBarakMode (){
  RunSomeMCMSPython $1 $2 Scripts/RunTest.sh Scripts/AllVersionTestsBarakMode.sh
}

#--DTD_EngineMRMUnitTests
DTD_EngineMRMUnitTests (){
  cd $ENGINE_DIR

  RunSmth $1 $2 ./mrm.sh test

  #RetCode=0
  #./mrm.sh test || RetCode=$? 
  #if [ $RetCode -ne 0 ]
  #then
  #  echo $COLORRED
  #  echo "EngineMRM unit tests failed: exitCode $RetCode"
  #  echo $TXTRESET 
  #  $1 $RetCode
  #fi
  
  #param=${2-false}
  #if [ $param == "true" ]
  #then
  #  $1 0
  #fi
}
