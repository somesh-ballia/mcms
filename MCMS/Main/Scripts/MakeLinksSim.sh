#!/bin/bash
SYS_PACK=$1
TOOLCHAIN=$2
BIN_RMX=$3
OPENSSL=openssl-1.0.1l
FLEXERA_DIR=/opt/polycom/flexera/fne-xt-i86_linux-5.2.0

ln -sf $SYS_PACK/net-snmp-5.7.2/agent/.libs/snmpd

cd $BIN_RMX

if [[ "$BIN_RMX" == "Bin.i32ptx" ]];then
ln -sf $SYS_PACK/httpd-2.4.7/.libs/httpd 
else
	ln -sf $SYS_PACK/SoftMCU/bin/httpd 
fi
ln -sf $SYS_PACK/httpd-2.4.7/srclib/apr-util/.libs/libaprutil-1.so.0
ln -sf $SYS_PACK/httpd-2.4.7/srclib/apr-util/.libs/libaprutil-1.so
ln -sf $SYS_PACK/httpd-2.4.7/srclib/apr/.libs/libapr-1.so.0
ln -sf $SYS_PACK/httpd-2.4.7/srclib/apr/.libs/libapr-1.so
ln -sf $SYS_PACK/httpd-2.4.7/srclib/apr-util/xml/expat/.libs/libexpat.so
ln -sf $SYS_PACK/httpd-2.4.7/srclib/apr-util/xml/expat/.libs/libexpat.so.0
ln -sf $SYS_PACK/httpd-2.4.7/srclib/apr-util/xml/expat/.libs/libexpat.so.0.5.0
ln -sf $SYS_PACK/httpd-2.4.7/srclib/pcre-8.20/.libs/libpcre.so.0
ln -sf $SYS_PACK/httpd-2.4.7/srclib/pcre-8.20/.libs/libpcre.so.0.0.1
ln -sf $SYS_PACK/apache-log4cxx-0.10.0/src/main/cpp/.libs/liblog4cxx.so liblog4cxx.so
ln -sf $SYS_PACK/apache-log4cxx-0.10.0/src/main/cpp/.libs/liblog4cxx.so liblog4cxx.so.10

ln -sf $SYS_PACK/libiconv-1.11/lib/.libs/libiconv.so.2.4.0
ln -sf $SYS_PACK/libiconv-1.11/lib/.libs/libiconv.so.2
ln -sf $SYS_PACK/libiconv-1.11/lib/.libs/libiconv.so
ln -sf $SYS_PACK/net-snmp-5.7.2/agent/.libs/libnetsnmpmibs.so.30.0.2
ln -sf $SYS_PACK/net-snmp-5.7.2/agent/.libs/libnetsnmpmibs.so.30
ln -sf $SYS_PACK/net-snmp-5.7.2/agent/.libs/libnetsnmpmibs.so
ln -sf $SYS_PACK/net-snmp-5.7.2/agent/.libs/libnetsnmpagent.so.30.0.2
ln -sf $SYS_PACK/net-snmp-5.7.2/agent/.libs/libnetsnmpagent.so.30
ln -sf $SYS_PACK/net-snmp-5.7.2/agent/.libs/libnetsnmpagent.so
ln -sf $SYS_PACK/net-snmp-5.7.2/agent/helpers/.libs/libnetsnmphelpers.so.30.0.2
ln -sf $SYS_PACK/net-snmp-5.7.2/agent/helpers/.libs/libnetsnmphelpers.so.30
ln -sf $SYS_PACK/net-snmp-5.7.2/agent/helpers/.libs/libnetsnmphelpers.so
ln -sf $SYS_PACK/net-snmp-5.7.2/snmplib/.libs/libnetsnmp.so.30.0.2
ln -sf $SYS_PACK/net-snmp-5.7.2/snmplib/.libs/libnetsnmp.so.30
ln -sf $SYS_PACK/net-snmp-5.7.2/snmplib/.libs/libnetsnmp.so
ln -sf $SYS_PACK/curl-7.32.0/lib/.libs/libcurl.so.4.3.0
ln -sf $SYS_PACK/curl-7.32.0/lib/.libs/libcurl.so.4
ln -sf $SYS_PACK/curl-7.32.0/lib/.libs/libcurl.so

# the trailer uses openssl for sha1 checksum
ln -sf $SYS_PACK/$OPENSSL/openssl
ln -sf $SYS_PACK/$OPENSSL/libssl.so.1.0.1 
ln -sf $SYS_PACK/$OPENSSL/libssl.so
ln -sf $SYS_PACK/$OPENSSL/libcrypto.so.1.0.1
ln -sf $SYS_PACK/$OPENSSL/libcrypto.so
ln -sf $SYS_PACK/zlib-1.2.7/libz.so.1.2.7
ln -sf $SYS_PACK/zlib-1.2.7/libz.so.1
ln -sf $SYS_PACK/zlib-1.2.7/libz.so


ln -sf $SYS_PACK/openldap-2.4.23/libraries/liblber/.libs/liblber.so
ln -sf $SYS_PACK/openldap-2.4.23/libraries/liblber/.libs/liblber-2.4.so.2
ln -sf $SYS_PACK/openldap-2.4.23/libraries/liblber/.libs/liblber-2.4.so.2.5.6
ln -sf $SYS_PACK/openldap-2.4.23/libraries/libldap/.libs/libldap.so
ln -sf $SYS_PACK/openldap-2.4.23/libraries/libldap/.libs/libldap-2.4.so.2
ln -sf $SYS_PACK/openldap-2.4.23/libraries/libldap/.libs/libldap-2.4.so.2.5.6
ln -sf $SYS_PACK/cyrus-sasl-2.1.22/lib/.libs/libsasl2.so
ln -sf $SYS_PACK/cyrus-sasl-2.1.22/lib/.libs/libsasl2.so.2
ln -sf $SYS_PACK/cyrus-sasl-2.1.22/lib/.libs/libsasl2.so.2.0.22
ln -sf $SYS_PACK/cyrus-sasl-2.1.22/plugins/.libs/libdigestmd5.so
ln -sf $SYS_PACK/cyrus-sasl-2.1.22/plugins/.libs/libdigestmd5.so.2
ln -sf $SYS_PACK/cyrus-sasl-2.1.22/plugins/.libs/libdigestmd5.so.2.0.22
ln -sf $SYS_PACK/cyrus-sasl-2.1.22/plugins/.libs/libanonymous.so
ln -sf $SYS_PACK/cyrus-sasl-2.1.22/plugins/.libs/libanonymous.so.2
ln -sf $SYS_PACK/cyrus-sasl-2.1.22/plugins/.libs/libanonymous.so.2.0.22
ln -sf $SYS_PACK/cyrus-sasl-2.1.22/plugins/.libs/libgssspnego.so
ln -sf $SYS_PACK/cyrus-sasl-2.1.22/plugins/.libs/libgssspnego.so.2
ln -sf $SYS_PACK/cyrus-sasl-2.1.22/plugins/.libs/libgssspnego.so.2.0.22

ln -sf $SYS_PACK/libxml2-2.9.0/.libs/libxml2.so
ln -sf $SYS_PACK/libxml2-2.9.0/.libs/libxml2.so.2
ln -sf $SYS_PACK/libxml2-2.9.0/.libs/libxml2.so.2.9.0


if [ "$VM" != "YES" ]; then
	ln -sf $TOOLCHAIN/sysroot-i686-unknown-linux-gnu/usr/lib/libresolv.so
	ln -sf $TOOLCHAIN/sysroot-i686-unknown-linux-gnu/usr/lib/libresolv.a
	ln -sf $TOOLCHAIN/sysroot-i686-unknown-linux-gnu/usr/lib/libm.so

fi

if [[ "$BIN_RMX" == "Bin.i32ptx" ]];then	
	ln -sf $TOOLCHAIN/i686-unknown-linux-gnu/lib/libstdc++.so.6.0.8
	ln -sf $TOOLCHAIN/i686-unknown-linux-gnu/lib/libstdc++.so.6
	ln -sf $TOOLCHAIN/i686-unknown-linux-gnu/lib/libstdc++.so
fi

if [[ "$BIN_RMX" != "Bin.i32ptx" ]];then	
	ln -sf $SYS_PACK/SoftMCU/modules/mod_rewrite.so
	ln -sf $SYS_PACK/SoftMCU/modules/libphp5.so
	ln -sf $SYS_PACK/SoftMCU/modules/mod_asis.so
fi

ln -sf $FLEXERA_DIR/lib/libFlxComm.so 
ln -sf $FLEXERA_DIR/lib/libFlxCore.so
