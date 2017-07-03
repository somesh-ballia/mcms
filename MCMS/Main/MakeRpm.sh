# run this script from vob/MCMS/Main
   
# Script arguments are:
# ---------------------
# $1 - rebuild (clean and make) before building rpm (yes/no)


MAKE_PATH=$PWD

export BUILD_PATH=$PWD

if [[ ! -f $BUILD_PATH/mcms_version ]]; then
        echo "0.0.0.0" > $BUILD_PATH/mcms_version
fi

#Edit Release header in spec file
Kernel=`uname -r | cut -d '-' -f 1`
# CentOS 5.8 
if [[ "$Kernel" == "2.6.18" ]]; then
        RELEASE="1.el5"
        SOURCE_BIN_FOLDER=Bin.i32cent56

# CentOs 6.3
elif [[ "$Kernel" == "2.6.32" ]]; then
        RELEASE="1.el6"
        SOURCE_BIN_FOLDER=Bin.i32cent6x
fi 

RPM_BUILD_DIR=$PWD/McmsRpmbuild_$RELEASE
rm -rf $RPM_BUILD_DIR

if test -e $BUILD_PATH/mcms_version; then
	echo -n "taking mcms version from file: mcms_version:"
	cat mcms_version
	BASELINE=`cat mcms_version`
else

	echo "taking mcms version from clear case"

	FOUNDATION_FIRST_LINE=`cleartool lsstream -l | grep -n "foundation baselines:" | cut -d':' -f1`
	RECOMMENDED_FIRST_LINE=`cleartool lsstream -l | grep -n "recommended baselines:" | cut -d':' -f1`
	VIEWS_FIRST_LINE=`cleartool lsstream -l | grep -n "views:" | cut -d':' -f1`
	DIFF_LINES=$(( VIEWS_FIRST_LINE - RECOMMENDED_FIRST_LINE ))

	if [ "$USER" != "builder" ]; then
		BASELINE=`cleartool lsstream -l | grep -A $DIFF_LINES "recommended baselines:" | grep MCMS | sed 's/.*MCMS-V/V/' | cut -d'@' -f1 | cut -d'V' -f2`
	else
		BASELINE=`cleartool lsbl -component _MCMS_BL@/vob/polycom_pvob -s -cview | tail -1 | cut -d'-' -f2 | cut -d'V' -f2`
	fi

	if [ "$BASELINE" == "" ]; then
		DIFF_LINES=$(( RECOMMENDED_FIRST_LINE - FOUNDATION_FIRST_LINE ))
	BASELINE=`cleartool lsstream -l | grep -A $DIFF_LINES "foundation baselines:" | grep MCMS | cut -d'@' -f1 | sed 's/.*MCMS-V/V/' |cut -d'V' -f2| head -n 1`
	fi

fi

if [ "$BASELINE" == "" ]; then
	BASELINE=`cat version.h | cut -f3 -d" " | sed 's/[ \t"]*//g' | sed 's/.*MCMS-V/V/' | cut -d'V' -f2 | head -n 1`
fi

echo "BASELINE="$BASELINE


if [ "$USER" != "builder" ]; then
	BASELINE="$BASELINE"_"$USER"_`date +%Y_%m_%d_%H:%M`
	rm -f $BUILD_PATH/mcms_version
fi


echo "RPM_NAME=$BASELINE"


#Verify 'clean' argument
if [ $# -eq 1 ]; then
	if [ "$1" == "yes" ]; then
		REBUILD_FLAG="yes"
	elif [ "$1" == "no" ]; then

		REBUILD_FLAG="no"
	else
		echo "Usage: argument must be [yes/no]"
		exit 1
	fi
else
	echo "Usage: must supply a rebuild argument [yes/no]"
	exit 1
fi

if [ "$REBUILD_FLAG" == "yes" ]; then
	./fast_clean.sh
	./make.sh
	if [[ $? != 0 ]]; then
		echo "MCMS compilation failed."
		exit 1
	fi
fi

export Filename=Plcm-Mcms
rm -rf $RPM_BUILD_DIR

mkdir -p $RPM_BUILD_DIR/rpmbuild/SOURCES
mkdir -p $RPM_BUILD_DIR/rpmbuild/SPECS
mkdir -p $RPM_BUILD_DIR/rpmbuild/RPMS
mkdir -p $RPM_BUILD_DIR/rpmbuild/SRPMS
mkdir -p $RPM_BUILD_DIR/rpmbuild/BUILD
mkdir -p $RPM_BUILD_DIR/rpmbuild/BUILDROOT

cp -f $Filename.spec $RPM_BUILD_DIR/rpmbuild/SPECS

#Edit Version header in spec file
SED_BASELINE=`echo $BASELINE | sed 's/\./\\./g' | sed 's/:/_/g' `
echo $SED_BASELINE
sed -i -e "s/^Version:.*/Version:\t$SED_BASELINE/" $RPM_BUILD_DIR/rpmbuild/SPECS/$Filename.spec


./Scripts/GenVersion.xml > Versions.xml


# todo:those files should be compiled to soft mcu...
cp  Bin.i32ptx/ApacheModule Bin.i32ptx/mod_polycom.so Bin.i32ptx/libMcmsCs.so Bin/

sed -i -e "s/^Release:.*/Release:\t$RELEASE/" $RPM_BUILD_DIR/rpmbuild/SPECS/$Filename.spec


cd ..
echo $PWD

#tar zcfh $RPM_BUILD_DIR/rpmbuild/SOURCES/$Filename.tar.gz Main/$SOURCE_BIN_FOLDER Main/Scripts Main/StaticCfg Main/MIBS Main/VersionCfg 
rm -rf $MCU_HOME_DIR/tmp/MCMS_tmp.$$
mkdir -p $MCU_HOME_DIR/tmp/MCMS_tmp.$$/Main
rsync -avrL --exclude=.svn Main/$SOURCE_BIN_FOLDER Main/snmpd Main/Scripts Main/StaticCfg Main/MIBS Main/VersionCfg Main/Versions.xml $MCU_HOME_DIR/tmp/MCMS_tmp.$$/Main
cd $MCU_HOME_DIR/tmp/MCMS_tmp.$$
tar zchf $RPM_BUILD_DIR/rpmbuild/SOURCES/$Filename.tar.gz Main 

cd $RPM_BUILD_DIR
rm -rf $MCU_HOME_DIR/tmp/MCMS_tmp.$$

# Creates RPMs with custom names.
PROJECTDIR=$RPM_BUILD_DIR/
SPECFILE=rpmbuild/SPECS/$Filename.spec
BUILDROOT=$PROJECTDIR/rpmbuild/BUILDROOT/
RPMNAMES="\
         Plcm \
         ibm-sametime-vmcu-9.0.0.0 \
         "

for n in $RPMNAMES
do
	
	# for ibm-sametime-vmcu-9.0.0.1, edit spec file	to:
	# - libgssspnego.so* are removed
	# LdapModule is removed
 	if [[ "ibm-sametime-vmcu-9.0.0.0" == ${n} ]];then
		sed -i "s/^#rm -f \.\.\/BUILDROOT\$MCU_HOME_DIR/mcms\/Bin\/libgssspnego.so\*$/rm -f \.\.\/BUILDROOT\$MCU_HOME_DIR/mcms\/Bin\/libgssspnego.so\*/" ${SPECFILE}
		sed -i "s/^#rm -f \.\.\/BUILDROOT\$MCU_HOME_DIR/mcms\/Bin\/LdapModule$/rm -f \.\.\/BUILDROOT\$MCU_HOME_DIR/mcms\/Bin\/LdapModule/" ${SPECFILE}
		sed -i "s/^Requires:.*openldap%{?_isa}/#Requires: openldap%{?_isa}/" ${SPECFILE}
	fi
  # Uses Plcm as placeholder.
  sed -i -e "s/Plcm/$n/" ${PROJECTDIR}${SPECFILE}

  rm -rf $BUILDROOT
  rpmbuild --target=i386 -bb --buildroot=$BUILDROOT $SPECFILE

  # Returns the placeholder.
  sed -i -e "s/$n/Plcm/" ${PROJECTDIR}${SPECFILE}
done
