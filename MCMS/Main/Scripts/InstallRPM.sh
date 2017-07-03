#!/bin/bash
# InstallRPM.sh
#
# Usage:
#   InstallRPM.sh --clean
#     Uninstall all Polycom RPMs.
#   InstallRPM.sh <path> <product>
#     Install or upgrade RPMs from the path.
#     If the path contains /CarmelVersion mounts /CarmelVersion locally.
#     Optional product: CG, Edge, Gesher, MFW

MCU_HOME_DIR=

LOG=$MCU_HOME_DIR/tmp/InstallRPM.log
NFS_SERVER=isrrndfs02.polycom.com

trap "echo Aborting the installation...; exit 1;" SIGHUP SIGINT SIGTERM


RHEL_AUX_PKG_LIST_84="\
  glibc \
  glibc.i686 \
  libgcc \
  libgcc.i686 \
  libusb \
  libusb.i686 \
  libxml2 \
  libxml2.i686 \
  zlib \
  zlib.i686 \
  libstdc++ \
  libstdc++.i686 \
  expat \
  freetype"

RHEL_AUX_PKG_LIST_81="\
  glibc \
  glibc.i686 \
  libgcc \
  libgcc.i686 \
  libusb \
  libusb.i686\
  libxml2 \
  libxml2.i686 \
  zlib \
  zlib.i686 \
  nss-softokn-freebl \
  nss-softokn-freebl.i686 \
  krb5-libs \
  krb5-libs.i686 \
  openssl \
  openssl.i686 \
  openssl098e \
  openssl098e.i686 \
  iptables \
  iptables.i686 \
  libstdc++ \
  libstdc++.i686 \
  compat-expat* \
  net-snmp \
  ksh"


SUSE_AUX_PKG_LIST="\
  libusb-0_1-4-32bit \
  openssl \
  net-snmp"


MCU_PKG_LIST_84="\
  *-Mcms-* \
  *-Rmx1000-* \
  *-VmpSoft-* \
  *-AmpSoft-* \
  *-MpProxy-* \
  *-UI_AIM-* \
  *-jsoncpp-* \
  *-EngineMRM-* \
  *-Cs-*|--nodeps \
  *-Ema-* \
  *-Mpmx-* \
  *-SingleApache-* \
  *-SoftMcuMainMFW-*"

MCU_PKG_LIST_81="\
  *-Mcms-* \
  *-Rmx1000-* \
  *-VmpSoft-* \
  *-AmpSoft-* \
  *-MpProxy-* \
  *-httpd-*|--nodeps \
  *-libphp-*|--nodeps \
  *-jsoncpp-* \
  *-UI_AIM-* \
  *-EngineMRM-* \
  *-Cs-*|--nodeps \
  *-Ema-* \
  *-Mpmx-* \
  *-SoftMcu*-*"

test_func()
{
  "$@"
  stat=$?
  if [ $stat -ne 0 ]; then
    echo "Error with $1" | tee -a $LOG
    exit 1
  fi

  return $stat
}

is_81()
{
  m1=$(echo "$1" | awk -F '.' '{print $1}')
  m2=$(echo "$1" | awk -F '.' '{print $2}')

  if [ "$m1" = 8 ] && [ "$m2" = 1 ]; then
    return 0 
  fi

  return 1
}

stop_and_clean()
{
  if [ ! "X$cur_ver" = "X" ]; then
    echo "Remove $cur_ver" | tee -a $LOG

    if is_81 $cur_ver; then
      pkgs=($MCU_PKG_LIST_81)
    else
      pkgs=($MCU_PKG_LIST_84)
    fi

    for ((i=${#pkgs[@]}-1; i>=0; i--)); do
      nme=`echo ${pkgs[$i]} | awk -F '|' '{ print $1 }'`

      # Extracts human package name.
      tmp=${nme#*-}
      man=${tmp%-*}
      pkg=$(rpm -qa | grep "\-$man")

      if [ ! "X$pkg" = "X" ]; then
        echo "Remove $pkg"
        rpm -e "$pkg" 2>&1 | tee -a $LOG

        if [ ${PIPESTATUS[0]} -ne 0 ]; then
          echo "Failed to remove package $nme" | tee -a $LOG
          return 1
        fi
      else
        echo "$man is not installed" | tee -a $LOG
      fi
    done
  else
     # Cleans them all.
     pkgs=($(rpm -qa | grep -e sametime -e Plcm))
     for ((i=${#pkgs[@]}-1; i>=0; i--)); do
       pkg="${pkgs[$i]}"
       echo "Remove $pkg"
       rpm -e "$pkg" 2>&1 | tee -a $LOG
       if [ ${PIPESTATUS[0]} -ne 0 ]; then
         echo "Failed to remove package $pkg" | tee -a $LOG
         return 1
       fi
     done
  fi

  # SUSE cleans itself automatically.
  if [ "$distro" = "RedHat" ]; then
    rm -rf /var/cache/yum/*
    yum clean all 2>&1 | tee -a $LOG
  fi

  return 0
}

set_src()
{
  # Mounts /Carmel-Versions if needed.
  carmel_dir="/Carmel-Versions"
  if echo "$rpm_dir" | grep -qv "$carmel_dir"; then
    return 0
  fi

  if mount | grep -q "$carmel_dir"; then
    echo "$carmel_dir" is mounted already | tee -a $LOG
    return 0
  fi

  mkdir -p "$carmel_dir"
  mount $NFS_SERVER:$MCU_HOME_DIR/data/Carmel-Versions/ "$carmel_dir" 2>&1 | tee -a $LOG

  if [ ${PIPESTATUS[0]} -ne 0 ]; then
    echo "Failed to mount $carmel_dir" | tee -a $LOG
    return 1
  fi

  echo "$carmel_dir" is mounted | tee -a $LOG
  return 0
}

set_repo()
{
  if [ ! "$distro" = "RedHat" ]; then
    return 0
  fi

  if [ "$ver" = "5.8" ]; then
    return 0
  fi

  repo_dir="/rhel-local-repo"
  if mount | grep -q "$repo_dir"; then
    echo "$repo_dir" is mounted already | tee -a $LOG
    return 0
  fi

  # Checks server availability.
  if ! ping -c 3 $NFS_SERVER > /dev/null 2>&1 ; then
    echo "$NFS_SERVER is unavailable, continue..." | tee -a $LOG
    return 0
  fi

  mkdir -p $repo_dir 2>&1 | tee -a $LOG
  mount -t nfs -o nfsvers=3 "$NFS_SERVER":/linux/data/nethome/mrm/rhel $repo_dir 2>&1 | tee -a $LOG
  if ! mount | grep $repo_dir > /dev/null 2>&1 ; then
    return 1
  fi

  echo "$repo_dir" is mounted | tee -a $LOG

  centos_repo="/etc/yum.repos.d/centos.repo"
  if test -f $centos_repo ; then
    mv $centos_repo $MCU_HOME_DIR/tmp/ 2>&1 | tee -a $LOG
  fi	

  rhel_local_repo="/etc/yum.repos.d/rhel-local.repo"
  echo "[rhel-6-local]" > $rhel_local_repo 2>&1 
  echo "name=RHEL-6-Local-Repo" >> $rhel_local_repo 2>&1
  echo "baseurl=file:///rhel-local-repo/6.4" >> $rhel_local_repo 2>&1
  echo "enabled=1" >> $rhel_local_repo 2>&1
  echo "gpgcheck=0" >> $rhel_local_repo 2>&1

  return 0
}

set_env()
{
  if test -f /etc/SuSE-release; then
    distro=SUSE
    yum_cmd='zypper -n in'
    USE_PKG_ARG=true
    ver=$(awk '{ if ($1 ~ /^VERSION/) print $3 }' /etc/SuSE-release)
  elif test -f /etc/redhat-release; then
    distro=RedHat
    yum_cmd="yum -y install"
    USE_PKG_ARG=false
    kernel=`uname -r | cut -d '-' -f 1`
    if [[ "$kernel" == "2.6.18" ]]; then
      ver="5.x"
    elif [[ "$kernel" == "2.6.32" ]]; then
      ver="6.x"
    else
      echo "RedHat version "$kernel" is unsupported" | tee -a $LOG
      return 1
   fi
  else
    echo "Unsupported Linux Distribution" | tee -a $LOG
    return 1
  fi

  cur_ver=$(rpm -qa | grep SoftMcu | awk -F '-' '{print $(NF-1)}')

  return 0
}

install_aux_pkgs()
{
  if [ "$ver" = "5.x" ]; then
    return 0
  fi

  $yum_cmd $AUX_PKG_LIST 2>&1 | tee -a $LOG
  if [ ${PIPESTATUS[0]} -ne 0 ]; then
    echo "Failed to install packages $AUX_PKG_LIST" | tee -a $LOG
    return 1 
  fi

  return 0
}

install_mcu_pkgs()
{
  for pkg in $MCU_PKG_LIST; do
    nme=`echo $pkg | awk -F '|' '{ print $1 }'`
    if $USE_PKG_ARG; then
      arg=`echo $pkg | awk -F '|' '{ print $2 }'`
    fi

    pkg_path="$rpm_dir"/"$nme"

    if echo "$nme" | grep -q SoftMcu; then
      if [ "$product" != "" ]; then
    	pkg_path="$rpm_dir"/"*-SoftMcuMain$product-*"
      fi
    fi

    pkg_file=`basename $pkg_path`

    # Extracts human package name.
    tmp=${nme#*-}
    man=${tmp%-*}
    pkg_cur_ver=$(rpm -qa | grep "\-$man\-" | awk -F '-' '{print $(NF-1)}')
    pkg_new_ver=$(echo $pkg_file | awk -F '-' '{print $(NF-1)}')

    if [[ "$pkg_cur_ver" = "$pkg_new_ver" && "$pkg_cur_ver" != "" ]]; then
      echo "$man $pkg_cur_ver is alredy exists, skipped" | tee -a $LOG
      continue
    fi

    if [ "X$pkg_cur_ver" = "X" ]; then
      echo "Installs $man $pkg_new_ver" | tee -a $LOG
    else
      echo "Upgrade $man from $pkg_cur_ver to $pkg_new_ver"
    fi

    rpm -Uv $pkg_path $arg 2>&1 | tee -a $LOG

    if [ ${PIPESTATUS[0]} -ne 0 ]; then
      echo "Failed to install package $man" | tee -a $LOG
      return 1
    fi
  done

  return 0
}

audit()
{
  local i=0
  while [ $i -le 5 ]
  do
    # Remove "is up" after it will be delivered to V100.
    service soft_mcu status | grep -qv -e "0" -e "is up" && echo "MCU service was brought up" | tee -a $LOG && return 1
    sleep 5
    i=`expr $i + 1`
  done

  # Remove $MCU_HOME_DIR/usr/share/SoftMcu/InstallValidator.sh after it will be delivered
  # to V100.
  if [ -f $MCU_HOME_DIR/mcms/Scripts/InstallValidator.sh ]; then
    $MCU_HOME_DIR/mcms/Scripts/InstallValidator.sh 2>&1 | tee -a $LOG
  elif [ -f $MCU_HOME_DIR/usr/share/SoftMcu/InstallValidator.sh ]; then
    $MCU_HOME_DIR/usr/share/SoftMcu/InstallValidator.sh 2>&1 | tee -a $LOG
  fi

  if test $? -ne 0; then
    echo "Install validator failed. Additional info is in $MCU_HOME_DIR/tmp/startup_logs/InstallValidator.log"
    return 1
  fi

  return 0
}

check_rpm_files()
{
  # Extracts version of a new Main RPM.
  if [ "$product" != "" ]; then
    	pat="*-SoftMcuMain$product-*"
  else
    pat="*-SoftMcu*"
  fi
  
  new_ver=$(find $rpm_dir -name "$pat" | awk -F '-' '{print $(NF-1)}')

  if is_81 $new_ver; then
    if [ "$distro" = "RedHat" ]; then
      AUX_PKG_LIST=$RHEL_AUX_PKG_LIST_81
    else
      AUX_PKG_LIST=$SUSE_AUX_PKG_LIST
    fi

    MCU_PKG_LIST=$MCU_PKG_LIST_81
    USE_PKG_ARG=true
  else
    if [ "$distro" = "RedHat" ]; then
      AUX_PKG_LIST=$RHEL_AUX_PKG_LIST_84
    else
      AUX_PKG_LIST=$SUSE_AUX_PKG_LIST
    fi

    MCU_PKG_LIST=$MCU_PKG_LIST_84
  fi

  ret=0
  for pkg in $MCU_PKG_LIST
  do
    nme=`echo $pkg | awk -F '|' '{print $1}'`

    if echo "$nme" | grep -q SoftMcu; then
      if [ "$product" != "" ]; then
    	nme="*-SoftMcuMain$product-*"
      fi
    fi

    num=`find "$rpm_dir" -name "$nme" | wc -l`

    # Extracts human package name.
    tmp=${nme#*-}
    nme=${tmp%-*}

    case "$num" in
    "0")
       echo "FAIL $rpm_dir/$nme" | tee -a $LOG
       ret=1
       ;;
    "1")
       echo "OKEY $rpm_dir/$nme" | tee -a $LOG
       ;;
    *)
       echo "FAIL $rpm_dir/$nme $num times" | tee -a $LOG
       ret=1
       ;;
    esac
  done

  return $ret
}

# Starts here.

# Cleans up log file.
> $LOG 2>/dev/null
echo "Start InstallRPM.sh $1" | tee -a $LOG

uid=$(id -u)
test $uid -ne 0 && echo "The script should be run as user 'root'" | tee -a $LOG && exit 1
test $# -lt 1 && echo "Missing argument: RPM directory or --clean" | tee -a $LOG && exit 1

test_func set_env
if [ "$1" = "--clean" ]; then
  test_func stop_and_clean
  exit 0
fi

rpm_dir=$1
product=$2

test_func set_src
test_func set_repo
[ ! -d "$rpm_dir" ] && echo "Directory "$rpm_dir" does not exists" | tee -a $LOG && exit 1

test_func check_rpm_files 

if [ "X$cur_ver" = "X" ]; then
  echo "Installs MCU $new_ver on $distro $ver" | tee -a $LOG
  test_func install_aux_pkgs
  test_func install_mcu_pkgs
else
  if [ "$cur_ver" = "$new_ver" ]; then
    echo "$new_ver is already installed" | tee -a $LOG
    exit 0
  fi
  
  echo "Upgrades MCU from $cur_ver to $new_ver on $distro $ver" | tee -a $LOG
  service soft_mcu stop 2>&1 | tee -a $LOG

  # Defines if it is major or minor version upgrade.
  cur_m1=$(echo "$cur_ver" | awk -F '.' '{print $1}')
  cur_m2=$(echo "$cur_ver" | awk -F '.' '{print $2}')
  new_m1=$(echo "$new_ver" | awk -F '.' '{print $1}')
  new_m2=$(echo "$new_ver" | awk -F '.' '{print $2}')

  if [ "$cur_m1" = "$new_m1" ] && [ "$cur_m2" = "$new_m2" ]; then
    # Minor version upgrade.

    if is_81 $cur_ver; then
      # Do not exist in 8.1.
      rpm -ev `rpm -qa | grep "\-libphp-"` --nodeps
      rpm -ev `rpm -qa | grep "\-httpd-"` --nodeps

      # Workaround.
      rpm -ev `rpm -qa | grep "\-Rmx1000-"` --nodeps
      rpm -ev `rpm -qa | grep "\-MpProxy-"` --nodeps
    else
      # Workaround.
      #rpm -ev `rpm -qa | grep "\-SingleApache-"` --nodeps
      echo no workaround
    fi
  else
    # Major version upgrade.

    # Allows only lower to higher.
    [ $cur_m1 -gt $new_m1 ] && echo "Upgrade from $cur_ver to $new_ver is not supported" | tee -a $LOG && exit 1
    [ $cur_m2 -gt $new_m2 ] && echo "Upgrade from $cur_ver to $new_ver is not supported" | tee -a $LOG && exit 1
    
    if is_81 $cur_ver; then
      # Do not exist in 8.1.
      rpm -ev `rpm -qa | grep "\-libphp-"` --nodeps
      rpm -ev `rpm -qa | grep "\-httpd-"` --nodeps

      # Workaround
      rpm -ev `rpm -qa | grep "\-Rmx1000-"` --nodeps
      rpm -ev `rpm -qa | grep "\-MpProxy-"` --nodeps
    fi
  fi

  test_func install_mcu_pkgs
fi

#This line must be right after the installation has ended
MCU_HOME_DIR=$(grep MCU_HOME_DIR /home/mcms/.bashrc | cut -d'=' -f2 | awk 'NR==1')

test_func audit

echo "Success" | tee -a $LOG

exit 0
