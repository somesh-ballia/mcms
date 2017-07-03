#/bin/sh

if [ ! $USER == "builder" ]
 then
   echo "#===============================================================================#"
   echo "# This Script Should Be executed by user builder In CM Build environment only!  #"
   echo "#         * Consult With CM Team Before modifying this Script.                  #"
   echo "#===============================================================================#"
   exit 1
fi

# START: CM Environemnt Prepare Phase

export PARENTCOMP="MCMS"
source ${SVN_PROD}/Jenkins/CM_set_env.sh

if [ -n "$RUNKW" ]
then
 if [ ${RUNKW} == "1" ]
 then
   echo "TAGPATH=${VERSION}/tags/${BUILD_TYPE}/${BRANCH}/${COMP}/${BUILD_NAME}" > environments.kw
   echo "TAG=${BUILD_NAME}" >> environments.kw
   echo "BUILDDIR=MCMS/MCMS/Main" >> environments.kw
   echo "BUILDCOMMAND=./make.sh" >> environments.kw
   echo "SKIP_TESTS=YES" >> environments.kw
   echo "SKIP_DEPEND=YES" >> environments.kw
   echo "PROJECT=MCMS" >> environments.kw
   cp -f environments.kw ../../
 fi
fi

# END: CM Environemnt Prepare Phase

# START: Compilation Phase

cd $WORKSPACE/$PARENTCOMP/Main


echo Create MCMS Ver file
echo -----------------

echo $WORKSPACE
cd $WORKSPACE/$PARENTCOMP/Main

export SKIP_TESTS=YES
export SKIP_DEPEND=YES

./fast_clean.sh
mkdir -p Bin.i32cent56 Bin.i32cent63 Bin.i32ptx
./make.sh || exit 1

#This must be very attached to MakeRPM.sh
echo -n $BUILD_NAME > mcms_version
cat mcms_version
echo $VIEW_OFFICIAL_PATH $BASELINE

./MakeRpm.sh no || exit 1

# END: Compilation Phase

# START: Create RPM and Copy outputs

echo Create and Copy RPM
#------------------------

mkdir -p $OUTBUILDPATH/$COMP/$BUILD_NAME/mcms
mkdir -p $OUTBUILDPATH/$COMP/$BUILD_NAME/RPM_IBM
mkdir -p $OUTBUILDPATH/$COMP/$BUILD_NAME/RPM


/opt/polycom/soft_mcu/RpmSigning/rpm_auto_sign.sh McmsRpmbuild_1.el*/rpmbuild/RPMS/i386/Plcm-Mcms*.*.rpm polycom
cp -dpRf McmsRpmbuild_1.el*/rpmbuild/RPMS/i386/Plcm-Mcms*.*.rpm $OUTBUILDPATH/$COMP/$BUILD_NAME/RPM

if (( ${OS} == "58" )); then
   /opt/polycom/soft_mcu/RpmSigning/rpm_auto_sign.sh McmsRpmbuild_1.el*/rpmbuild/RPMS/i386/ibm*.*.rpm polycom
   cp -dpRf McmsRpmbuild_1.el*/rpmbuild/RPMS/i386/ibm*.*.rpm $OUTBUILDPATH/$COMP/$BUILD_NAME/RPM_IBM


   rm -Rf `find $WORKSPACE/MCMS/Main/Bin.i32cent56 | grep "\\.svn$"`
   rm -Rf`find $WORKSPACE/MCMS/Main/Bin.i32cent56 | grep "\\.o$"`
   cp -dpRf $WORKSPACE/${PARENTCOMP}/Main/Bin.i32cent56 $OUTBUILDPATH/$COMP/$BUILD_NAME/mcms

fi



if (( ${OS} == "65" )); then
    echo "Compile Multilang"
    #-----------------
    mkdir -p /Carmel-Versions/SVN/MultilingualFiles/$VERSION/$BUILD_NAME
    (export LD_LIBRARY_PATH=$WORKSPACE/MCMS/Main/Bin.i32ptx/ ; export RMX_STATUS_DIR=/Carmel-Versions/SVN/MultilingualFiles/$VERSION/$BUILD_NAME ; $WORKSPACE/MCMS/Main/Bin.i32ptx/McuMngr)

    cd /Carmel-Versions/SVN/MultilingualFiles/$VERSION
    rm -f last
    ln -sf $BUILD_NAME last
    cd -

    echo Copy MCMS
    #------------------------
     echo "Remove all obj files from RMX view"

     cd $WORKSPACE/MCMS
     rm `find Main | grep "\\.o$"`
     rm -Rf `find Main | grep "\\.svn$"`
     rm -Rf Main/McmsRpmbuild*
     rm -Rf Main/ExternalProjects
     rm -Rf Main/Bin.i32cent56 

     cp -dpRvfu $WORKSPACE/${PARENTCOMP}/Main/* $OUTBUILDPATH/${COMP}/$BUILD_NAME/mcms
    
     echo "check rpm signature"
     rpm --nopgp --checksig $OUTBUILDPATH/${COMP}/$BUILD_NAME/*RPM*/*.rpm
fi
# END: Create RPM and Copy outputs
