
KERNELVERSION=$(shell uname -r | cut -d '-' -f 1)
#is CentOS-6.3

ifeq ($(KERNELVERSION),2.6.32)
SOFT_PLATFORM = i32cent6x
else
SOFT_PLATFORM = i32cent56
endif



RMX_PLATFORM = i32ptx

PLATFORMS = $(RMX_PLATFORM) $(SOFT_PLATFORM)
BIN_RMX = Bin.$(RMX_PLATFORM)
BIN_SOFT = Bin.$(SOFT_PLATFORM)


##################
### SOFT MCU #####
##################


CC_SOFT	 = /usr/bin/gcc
CXX_SOFT = /usr/bin/g++


CPPUNITLIB_SOFT = /usr/lib/libcppunit.a

ifeq ($(wildcard /usr/lib/libcppunit.a), ) 
	CPPUNITLIB_SOFT = -lcppunit
else 
	CPPUNITLIB_SOFT = /usr/lib/libcppunit.a
endif

SYS_PACK_SOFT = /opt/polycom/sim_pack/mcms_100.0_v016/
SYS_PACK_RMX = /opt/polycom/sim_pack/mcms_100.0_v016/

OPENSSL_VER=openssl-1.0.1l

ZLIBINC_SOFT = -I $(SYS_PACK_SOFT)/zlib-1.2.7
ZLIBLIB_SOFT = -L $(SYS_PACK_SOFT)/zlib-1.2.7

HTTPINC_SOFT = -I $(SYS_PACK_SOFT)/httpd-2.4.7/include \
          -I $(SYS_PACK_SOFT)/httpd-2.4.7/srclib/apr-util/include/ \
          -I $(SYS_PACK_SOFT)/httpd-2.4.7/srclib/apr/include/ \
          -I $(SYS_PACK_SOFT)/httpd-2.4.7/os/unix


CPPUNITINC_SOFT = -I Libs/Mocks -I /usr/local/include 
 
ENCRYPTINC_SOFT = -I Libs/McmsEncryption -I $(SYS_PACK_SOFT)$(OPENSSL_VER)/include/ 

ENCRYPTLIB_SOFT = -L $(BIN_SOFT) -L $(SYS_PACK_SOFT)/$(OPENSSL_VER) -lssl -lcrypto -lMcmsEncryption

RESOLVLIB_SOFT = /usr/lib/libresolv.a

CURLINC_SOFT = -I $(SYS_PACK_SOFT)/curl-7.32.0/include -I Libs/McmsCurlLib $(ENCRYPTINC_SOFT)


LDAPLINC_SOFT = -I $(SYS_PACK_SOFT)openldap-2.4.23/include \
	  	   -I $(SYS_PACK_SOFT)cyrus-sasl-2.1.22/include -I /usr/include/sasl

LDAPLIB_SOFT = -L $(SYS_PACK_SOFT)/openldap-2.4.23/libraries/libldap/.libs/ -lldap \
		  -L $(SYS_PACK_SOFT)/openldap-2.4.23/libraries/liblber/.libs/ -llber \
		  -L $(SYS_PACK_SOFT)/cyrus-sasl-2.1.22/lib/.libs/ -lsasl2



LOG4CXXINC_SOFT = -I $(SYS_PACK_SOFT)/apache-log4cxx-0.10.0/src/main/include/

LOG4CXXLIB_SOFT = -L $(SYS_PACK_SOFT)/apache-log4cxx-0.10.0/src/main/cpp/.libs/ -llog4cxx \
			-L $(SYS_PACK_SOFT)/httpd-2.4.7/srclib/apr/.libs/ -lapr-1 \
			-L $(SYS_PACK_SOFT)/httpd-2.4.7/srclib/apr-util/.libs/ -laprutil-1 \
			-L $(SYS_PACK_SOFT)/httpd-2.4.7/srclib/apr-util/xml/expat/.libs/ -lexpat \
			-L $(SYS_PACK_SOFT)/libiconv-1.11/lib/.libs/ -liconv


SNMPINC_SOFT = -I Libs/SNMPLib -DNETSNMP_NO_INLINE \
          -I $(SYS_PACK_SOFT)/net-snmp-5.7.2/include/net-snmp/ \
          -I $(SYS_PACK_SOFT)/net-snmp-5.7.2/agent/mibgroup/util_funcs/


SNMPLIB_SOFT = -lSNMP \
	      -L $(SYS_PACK_SOFT)/net-snmp-5.7.2/agent/.libs/ -lnetsnmpagent \
		  -L $(SYS_PACK_SOFT)/net-snmp-5.7.2/agent/.libs/ -lnetsnmpmibs \
	      -L $(SYS_PACK_SOFT)/net-snmp-5.7.2/agent/helpers/.libs/ -lnetsnmphelpers \
		  -L $(SYS_PACK_SOFT)/net-snmp-5.7.2/snmplib/.libs/ -lnetsnmp

CURLLIB_SOFT = -L $(SYS_PACK_SOFT)/$(OPENSSL_VER) -lssl -lcrypto -L $(SYS_PACK_RMX)/curl-7.32.0/lib/.libs/ -lcurl 

MCSMCURLLIB_SOFT = -lMcmsCurlLib


NDMLIB_SOFT = -L $(BIN_SOFT) -lNDM
#############
### RMX #####
#############

TOOLCHAIN_RMX = /opt/polycom/OSELAS.Toolchain-1.99.3_64bit/OSELAS.Toolchain-1.99.3/i686-unknown-linux-gnu/gcc-4.3.2-glibc-2.8-binutils-2.18-kernel-2.6.24/
CROSS_COMPILE_RMX = $(TOOLCHAIN_RMX)/bin/i686-unknown-linux-gnu-

CC_RMX	 = $(CROSS_COMPILE_RMX)gcc
CXX_RMX	 = $(CROSS_COMPILE_RMX)g++


ZLIBINC_RMX = -I $(SYS_PACK_RMX)/zlib-1.2.7


CPPUNITINC_RMX = -I Libs/Mocks \
		     -I $(SYS_PACK_RMX)/cppunit-1.12.1/include

SNMPINC_RMX = -I Libs/SNMPLib -DNETSNMP_NO_INLINE \
          -I $(SYS_PACK_RMX)/net-snmp-5.7.2/include/ \
          -I $(SYS_PACK_RMX)/net-snmp-5.7.2/agent/mibgroup/util_funcs/

ENCRYPTINC_RMX = -I Libs/McmsEncryption -I $(SYS_PACK_RMX)$(OPENSSL_VER)/include/

HTTPINC_RMX = -I $(SYS_PACK_RMX)/httpd-2.4.7/include \
	  -I $(SYS_PACK_RMX)/httpd-2.4.7/srclib/apr-util/include/ \
	  -I $(SYS_PACK_RMX)/httpd-2.4.7/srclib/apr/include/ \
	  -I $(SYS_PACK_RMX)/httpd-2.4.7/os/unix

CURLINC_RMX = -I $(SYS_PACK_RMX)/curl-7.32.0/include -I Libs/McmsCurlLib $(ENCRYPTINC_SOFT)

LDAPLINC_RMX = -I $(SYS_PACK_RMX)openldap-2.4.23/include \
	  	       -I $(SYS_PACK_RMX)cyrus-sasl-2.1.22/include

LOG4CXXINC_RMX = -I $(SYS_PACK_RMX)/apache-log4cxx-0.10.0/src/main/include/
CPPUNITLIB_RMX = $(SYS_PACK_RMX)/cppunit-1.12.1/src/cppunit/.libs/libcppunit.a
RESOLVLIB_RMX = $(TOOLCHAIN_RMX)/sysroot-i686-unknown-linux-gnu/usr/lib/libresolv.a

LDAPLIB_RMX = -lldap -llber -lsasl2

LOG4CXXLIB_RMX =  -llog4cxx -lapr-1 -laprutil-1 -lexpat \
	-L $(SYS_PACK_RMX)/libiconv-1.11/lib/.libs/ -liconv

SNMPLIB_RMX = -lSNMP  -L $(SYS_PACK_SOFT)/net-snmp-5.7.2/agent/.libs/ -lnetsnmpagent \
		  -L $(SYS_PACK_SOFT)/net-snmp-5.7.2/agent/.libs/ -lnetsnmpmibs \
	      -L $(SYS_PACK_SOFT)/net-snmp-5.7.2/agent/helpers/.libs/ -lnetsnmphelpers \
		  -L $(SYS_PACK_SOFT)/net-snmp-5.7.2/snmplib/.libs/ -lnetsnmpagent -lnetsnmpmibs -lnetsnmphelpers  -lnetsnmp

ENCRYPTLIB_RMX = -L $(BIN_RMX) -L $(SYS_PACK_RMX)/$(OPENSSL_VER) -lssl -lcrypto -lMcmsEncryption

STDLIB_RMX = -L$(CROSS_COMPILE_BASE_RMX)lib/ -L$(CROSS_COMPILE_BASE_RMX)i686-polycom-linux-gnu/lib/
STDLIB_SOFT = -L/usr/lib/libstdc++.so.6

CURLLIB_RMX = -L $(SYS_PACK_RMX)/$(OPENSSL_VER) -lssl -lcrypto -L $(SYS_PACK_RMX)/curl-7.32.0/lib/.libs/ -lcurl 

MCSMCURLLIB_RMX = -lMcmsCurlLib
#######  --- libxml is the same for Soft MCU and RMX Dist we allign it to be linked to the network place
XML2INC = -I $(SYS_PACK_RMX)/libxml2-2.9.0/include/
XML2LIB = -L $(SYS_PACK_RMX)/libxml2-2.9.0/.libs/ -lxml2
NDMLIB_RMX = -L $(BIN_RMX) -lNDM

FLEXERA_DIR = /opt/polycom/flexera/fne-xt-i86_linux-5.2.0
FLEXERALIB_SOFT = -L$(FLEXERA_DIR)/lib -lFlxClient
FLEXERAINC_SOFT = -I$(FLEXERA_DIR)/include 
