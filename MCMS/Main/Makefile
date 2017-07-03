 
.PHONY : clean test test_bin links sim active all version test_scripts makefiles automatic_sources

XML2SOURCE_BIN=Utils/Xml2Source/Xml2Source

include Main.mk
include Processes/CommonProcessInclude.mk


DISTCC   = $(DISTCC_BIN) $(CC)
DISTCXX  = $(DISTCC_BIN) $(CXX)

DISTCC_RMX    = $(DISTCC_BIN) $(CC_RMX)
DISTCXX_RMX  =  $(DISTCC_BIN) $(CXX_RMX)

DISTCC_SOFT    = $(DISTCC_BIN) $(CC_SOFT)
DISTCXX_SOFT  = $(DISTCC_BIN) $(CXX_SOFT)

# -Wno-write-strings  
# 
# In C++, nonzero means warn about deprecated
# conversion from string literals to `char *'.  In
# C, similar warning, except that the conversion is
# of course not deprecated by the ISO C standard.
#
# example : char * buffer = "hello world"

# -Wno-unused-function 
# Warn when a function is unused
# example : static int	FUNCTION_NAME(){;}

# -Wno-unused-variable
# Warn when a variable is unused
# error: 'StringValidityStatuses' defined but not used

# -Wno-unused-label
# Warn when a label is unused

# -Wno-unused
# Enable all -Wunused- warnings

WARNING_IGNORE =  -Wno-write-strings \
				  -Wno-unused-function \
				  -Wno-unused-variable \
				  -Wno-unused-label \
			      -Wno-unused

C_WARNING_IGNORE =   
CPP_WARNING_IGNORE = 

FLAGS    =  -fPIC -pipe -pthread -Wall -Wextra -Werror -fstack-protector -O0 -g -m32 -march=i386 -DLINUX -rdynamic $(MORE_FLAGS) $(WARNING_IGNORE) 

# Compiles with SSH on for debug

ifdef SSH_ON_FOR_DEBUG
FLAGS += -DSSH_ON_FOR_DEBUG
endif


# Avoid EOVERFLOW and O_LARGEFILE errors on onen a file whose size
# exceeds (2<<31)-1 bits
FLAGS += -D_FILE_OFFSET_BITS=64


CFLAGS   = $(FLAGS)
CFLAGS_RMX   = $(CFLAGS) $(C_WARNING_IGNORE)
CFLAGS_SOFT   = $(CFLAGS) $(C_WARNING_IGNORE)

CXXFLAGS = $(FLAGS)
CXXFLAGS_RMX = $(FLAGS) $(CPP_WARNING_IGNORE)
CXXFLAGS_SOFT = $(FLAGS) $(CPP_WARNING_IGNORE)

LINK     = $(CXX)
LINK_RMX     = $(CXX_RMX)
LINK_SOFT    = $(CXX_SOFT)

LFLAGS   = -m32 -rdynamic -fatal-warnings
LFLAGS_RMX   = $(LFLAGS)
LFLAGS_SOFT   = $(LFLAGS)

LIBS     = $(SUBLIBS) -lm 
AR       = $(CROSS_COMPILE)ar cqs
AR_RMX       = $(CROSS_COMPILE_RMX)ar cqs
AR_SOFT      = ar cqs
COPY     = cp -f
COPY_FILE= $(COPY)
COPY_DIR = $(COPY) -r
INSTALL_FILE= $(COPY_FILE)
INSTALL_DIR = $(COPY_DIR)
DEL_FILE = rm -f
SYMLINK  = ln -sf
DEL_DIR  = rmdir
MOVE     = mv -f
CHK_DIR_EXISTS= test -d
MKDIR    = mkdir -p
#MAKE = clearmake -C gnu
MAKE = make

ifeq ($(ENABLE_VERBOSE_OUTPUT),YES)
	Q=
else
	Q=@
endif



 
######################
first: all

####### Implicit rules

.SUFFIXES: .c .$(RMX_PLATFORM).o .$(SOFT_PLATFORM).o .cpp .depend

PROCESSES = $(shell ls -1 Processes | grep -v "\.")

-include Processes/*.depend

MAINCPP=Processes/Main.cpp
MAINOBJ_RMX=Processes/Main.$(RMX_PLATFORM).o
MAINOBJ_SOFT=Processes/Main.$(SOFT_PLATFORM).o


ifndef SKIP_TESTS

TESTMAIN=Processes/TestMain.cpp
TESTMAINOBJ=Processes/TestMain.$(SOFT_PLATFORM).o

endif

$(MAINOBJ_RMX): $(MAINCPP)
	@echo Building $@
	$(Q)$(DISTCXX_RMX) -c $(CXXFLAGS_RMX) $(COMMONINCPATH) -o $@ $<

$(MAINOBJ_SOFT): $(MAINCPP)
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) $(COMMONINCPATH) -o $@ $<

#	$(Q)$(CXX) -MM -MF $(patsubst %.o,%.depend,$@) $(COMMONINCPATH) $(CXXFLAGS)  $<

ifndef SKIP_TESTS

$(TESTMAINOBJ): $(TESTMAIN)
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) $(COMMONINCPATH) $(CPPUNITINC_SOFT) -o $@ $<
	$(Q)$(CXX_SOFT) -MM -MF $(patsubst %.o,%.depend,$@) $(COMMONINCPATH) $(CPPUNITINC_SOFT) $(CXXFLAGS_SOFT)  $<

endif

BASE_LIBS= -lrt  -lProcessBase -lXmlPars -lCommon -lSimLinux $(XML2LIB)
BASE_LIBS_RMX = -L$(BIN_RMX) $(BASE_LIBS) -L$(SYS_PACK_RMX)/zlib-1.2.7/ -lz
BASE_LIBS_SOFT = -L$(BIN_SOFT) $(BASE_LIBS) -L$(SYS_PACK_RMX)/zlib-1.2.7/ -lz
BASE_DEPEND_RMX = $(BIN_RMX)/libProcessBase.so $(BIN_RMX)/libXmlPars.so $(BIN_RMX)/libCommon.so $(BIN_RMX)/libSimLinux.so
BASE_DEPEND_SOFT = $(BIN_SOFT)/libProcessBase.so $(BIN_SOFT)/libXmlPars.so $(BIN_SOFT)/libCommon.so $(BIN_SOFT)/libSimLinux.so



makefiles:
	@for process in $(shell ls Processes); do  Scripts/GenMake.sh $$process 2>/dev/null > Processes/$$process/Makefile.auto  || echo -n ; done;
 

ifneq ($(MAKECMDGOALS) , makefiles)

include Libs/*/Makefile
-include Processes/*/Makefile.auto
#include Utils/*/*/Makefile
include Utils/*/Makefile
include Processes/ApacheModule/Makefile
include Processes/IPMCInterface/Makefile
include Processes/CsModule/Makefile

endif

automatic_sources: $(PREREQUIRED_SOURCES)


all: active targets


#	@$(MAKE) active
#	@$(MAKE) targets



targets:	$(TARGETS_RMX) $(TARGETS_SOFT) $(TEST_TARGETS)

rmx:	active $(TARGETS_RMX) 

soft:	$(TARGETS_SOFT) 


clean:
	@echo Performing $@
	find .  -name *.o -exec rm -f {} \;
	find .  -name *.a | grep -v /Utils/Image2IVRSlides/libvslide/lib/ | xargs rm -f \;
	rm -f Bin/*
	rm -f Stripped/*
	rm -f LogFiles/*
	rm -f CdrFiles/*
	rm -f *core*
	rm -f Cfg/Hlog/*
	rm -Rf TestResults/*
	rm -f httpd.access.log
	rm -Rf /tmp/EMACfg/*
	rm -Rf /tmp/OCS/*
	rm -Rf /tmp/Install/*
	rm -Rf MediaRecording/share/*
	rm -f Bin.*/*
	rm -f Processes/*/Makefile.auto
	rm -f bin rpm
	rm -Rf TS


mrproper: clean
	@echo Performing $@ 
	find .  -name *.depend -exec rm -f {} \;
	find .  -name *.flag -exec rm -f {} \;
	rm -Rf /tmp/ccache_dir/*
	@Scripts/CleanAutoXmlGeneratedFiles.sh

ifndef SKIP_TESTS

test_bin: $(TEST_TARGETS)



test: test_bin
	@Scripts/RunMakeTests.sh


unitests:
	@$(MAKE)  all
	@Scripts/Cleanup.sh
	@$(MAKE)  test
	export LD_LIBRARY_PATH=/mcms/Bin ; \
	export SASL_PATH=/mcms/Bin ; \
	./Bin/CommonLib.Test ; 
		export LD_LIBRARY_PATH=/mcms/Bin ; \
		export SASL_PATH=/mcms/Bin ; \

endif

rebuild: clean all active test

active: links sim

sim:
	@rm -Rf /tmp/queue/
	@rm -Rf /tmp/shared_memory/
	@rm -Rf /tmp/semaphore
	@rm -Rf /tmp/apache/
	@rm -Rf /tmp/Cfg/
	@rm -Rf /tmp/OCS/
	@rm -f /tmp/Bin
	@rm -Rf /tmp/Install
	@rm -Rf LOGFILES
	@rm -Rf /tmp/EMACfg/
	-@rm -Rf $(PWD)/TestResults
	@rm -f /tmp/Cfg/ema
	@rm -Rf /tmp/class/
	@mkdir /tmp/queue/
	@mkdir /tmp/shared_memory/
	@mkdir /tmp/apache/
	@mkdir /tmp/Cfg/
	@mkdir /tmp/OCS/
	@mkdir /tmp/EMACfg/
	@touch /tmp/EMACfg/EMA.DataObjects.OfflineTemplates.AddressbookContent_.xml
	@mkdir /tmp/semaphore/
	-@mkdir $(PWD)/TestResults
	@mkdir /tmp/Install
	@ln -s $(PWD)/Bin/mod_polycom.so /tmp/apache/mod_polycom.so
	@ln -s $(PWD)/StaticCfg/ActionRedirection.xml /tmp/apache/ActionRedirection.xml
	@ln -s $(PWD)/Bin /tmp/Bin
	@rm -f /tmp/mcms
	@ln -s $(PWD) /tmp/mcms
	@rm -f /mcms/EMA/Versions.xml
	@mkdir -p Cfg
	@touch $(PWD)/Cfg/Versions.xml
	@ln -s $(PWD)/Cfg/Versions.xml /mcms/EMA/Versions.xml
	@ln -sf Scripts/McmsConnection.py
	@ln -sf Scripts/McmsTargetConnection.py
	@rm -f python
	@ln -sf /opt/polycom/Python-2.7.6 python
#	Important : please do not remove this link to pyton it causes failure in teh Eclipse enviroemnt
#	@ln -sf /usr python
	@mkdir /tmp/class
	@mkdir /tmp/class/hwmon
	@mkdir /tmp/class/hwmon/hwmon0
	@mkdir /tmp/class/hwmon/hwmon0/device
	@touch /tmp/class/hwmon/hwmon0/device/temp1_input	
	@echo 35000 >>/tmp/class/hwmon/hwmon0/device/temp1_input
	@touch /tmp/class/hwmon/hwmon0/device/temp1_label	
	@echo Core 0 >>/tmp/class/hwmon/hwmon0/device/temp1_label
	@mkdir /tmp/class/hwmon/hwmon1
	@mkdir /tmp/class/hwmon/hwmon1/device
	@touch /tmp/class/hwmon/hwmon1/device/temp1_input
	@touch /tmp/class/hwmon/hwmon1/device/temp1_label	
	@echo Core 1 >>/tmp/class/hwmon/hwmon1/device/temp1_label
	@echo 37000 >>/tmp/class/hwmon/hwmon1/device/temp1_input	

links:
	@mkdir -p Bin.$(RMX_PLATFORM) Bin.$(SOFT_PLATFORM) 
	@mkdir -p Stripped
	@Scripts/BinLinks.sh
	@Scripts/MakeLinksSim.sh $(SYS_PACK_RMX) $(TOOLCHAIN_RMX) $(BIN_RMX)
	@Scripts/MakeLinksSim.sh $(SYS_PACK_SOFT) $(TOOLCHAIN_RMX) $(BIN_SOFT)
	@cd $(BIN_RMX) ; ln -sf $(SYS_PACK_RMX)/net-snmp-5.7.2/agent/.libs/libnetsnmpmibs.so.30.0.2
	@cd $(BIN_RMX) ; ln -sf $(SYS_PACK_RMX)/net-snmp-5.7.2/agent/.libs/libnetsnmpmibs.so.30
	@cd $(BIN_RMX) ; ln -sf $(SYS_PACK_RMX)/net-snmp-5.7.2/agent/.libs/libnetsnmpmibs.so
	@cd $(BIN_RMX) ; ln -sf $(SYS_PACK_RMX)/net-snmp-5.7.2/agent/.libs/libnetsnmpagent.so.30.0.2
	@cd $(BIN_RMX) ; ln -sf $(SYS_PACK_RMX)/net-snmp-5.7.2/agent/.libs/libnetsnmpagent.so.30
	@cd $(BIN_RMX) ; ln -sf $(SYS_PACK_RMX)/net-snmp-5.7.2/agent/.libs/libnetsnmpagent.so
	@cd $(BIN_RMX) ; ln -sf $(SYS_PACK_RMX)/net-snmp-5.7.2/agent/helpers/.libs/libnetsnmphelpers.so.30.0.2
	@cd $(BIN_RMX) ; ln -sf $(SYS_PACK_RMX)/net-snmp-5.7.2/agent/helpers/.libs/libnetsnmphelpers.so.30
	@cd $(BIN_RMX) ; ln -sf $(SYS_PACK_RMX)/net-snmp-5.7.2/agent/helpers/.libs/libnetsnmphelpers.so
	@cd $(BIN_RMX) ; ln -sf $(SYS_PACK_RMX)/net-snmp-5.7.2/snmplib/.libs/libnetsnmp.so.30.0.2
	@cd $(BIN_RMX) ; ln -sf $(SYS_PACK_RMX)/net-snmp-5.7.2/snmplib/.libs/libnetsnmp.so.30
	@cd $(BIN_RMX) ; ln -sf $(SYS_PACK_RMX)/net-snmp-5.7.2/snmplib/.libs/libnetsnmp.so
	@ln -sf $(SYS_PACK_RMX)/net-snmp-5.7.2/agent/.libs/snmpd
	@rm -Rf	$(PWD)/VersionCfg/IVR/font_hash.md5
	@cd $(PWD)/VersionCfg/IVR/Fonts/; find -type f -exec md5sum {} \; >> ../font_hash.md5;cd -;
	@ln -sf $(SYS_PACK_RMX)/zlib-1.2.7/libz.so.1 $(BIN_RMX)/libz.so

ifndef SKIP_TESTS
version:
	@$(MAKE) all
	@Scripts/Cleanup.sh
	@$(MAKE)  test
	@Scripts/Cleanup.sh	
	@$(MAKE)  test_scripts

ver:
	@./make.sh
	@Scripts/Cleanup.sh
	@$(MAKE)  test
	@Scripts/Cleanup.sh
	@$(MAKE)  test_scripts

versionISDN:
	@$(MAKE)  all
	@Scripts/Cleanup.sh	
	@$(MAKE)  ISDN_test_scripts

ISDN_test_scripts:
#	-@Scripts/RunTest.sh Scripts/ISDNConf.py 
	  Scripts/RunTest.sh Scripts/ISDN_DialIn.py && \
	  Scripts/RunTest.sh Scripts/ISDN_DialOut.py && \
	  Scripts/RunTest.sh Scripts/ISDN_MultiRate.py && \
	  Scripts/RunTest.sh Scripts/ISDN_MultyTypeConf.py && \
	  Scripts/RunTest.sh Scripts/ISDN_Move.py && \
	  Scripts/RunTest.sh Scripts/ISDN_DialInDialOut.py && \
	  Scripts/RunTest.sh Scripts/ISDN_ReconnectParty.py && \
	  Scripts/RunTest.sh Scripts/ISDN_MaxCapacity.py && \
	  Scripts/RunTest.sh Scripts/ISDN_ConfVideoLayout.py
#	-@Scripts/RunTest.sh Scripts/ISDN_MaxDialInCapacity.py ConfParty

test_scripts:
	export LD_LIBRARY_PATH=/mcms/Bin ; \
	export SASL_PATH=/mcms/Bin ; \
	USE_ALT_IP_SERVICE=VersionCfg/DefaultIPServiceListWithDNSWithProxyAndGk.xml Scripts/RunTest.sh Scripts/AllVersionTestsBreezeModeCP.sh  $(VALGRIND_PROCESS_FOR_STATIC)

all_test_scripts:
	  Scripts/RunTest.sh Scripts/AutoRealVoip.sh && \
	  Scripts/RunTest.sh Scripts/AutoRealVideo.sh && \
	  Scripts/RunTest.sh Scripts/Add20ConferenceNew.sh && \
	  Scripts/RunTest.sh Scripts/AddDeleteIpServ.sh && \
	  Scripts/RunTest.sh Scripts/AddDeleteNewIvr.sh && \
	  Scripts/RunTest.sh Scripts/AddIpServiceWithGkNew.sh && \
	  Scripts/RunTest.sh Scripts/AddIpServiceWithGk.sh && \
	  Scripts/RunTest.sh Scripts/AddRemoveMrNew.sh && \
	  Scripts/RunTest.sh Scripts/AddRemoveOperator.sh && \
	  Scripts/RunTest.sh Scripts/AddRemoveProfile.sh

pstn_tests:
	  Scripts/RunTest.sh Scripts/PSTN_1.py && \
	  Scripts/RunTest.sh Scripts/PSTN_2.py && \
	  Scripts/RunTest.sh Scripts/PSTN_3.py && \
	  Scripts/RunTest.sh Scripts/PSTN_4.py && \
	  Scripts/RunTest.sh Scripts/PSTN_5.py && \
	  Scripts/RunTest.sh Scripts/PSTN_6.py && \
	  Scripts/RunTest.sh Scripts/PSTN_Move.py && \
	  Scripts/RunTest.sh Scripts/PSTN_MaxCapcity.py && \
	  Scripts/RunTest.sh Scripts/PSTN_DialIn.py && \
	  Scripts/RunTest.sh Scripts/AdHocGW.py && \
	  Scripts/RunTest.sh Scripts/PSTN_5.py ConfParty && \
	  Scripts/RunTest.sh Scripts/PSTN_6.py ConfParty && \
	  Scripts/RunTest.sh Scripts/PSTN_Move.py ConfParty && \
	  Scripts/RunTest.sh Scripts/PSTN_DialIn.py ConfParty && \
	  Scripts/RunTest.sh Scripts/AdHocGW.py ConfParty


test_night:
#	-@Scripts/RunTest.sh Scripts/AutoRealVoip.sh valgrind
	Scripts/RunTest.sh Scripts/AutoRealVideo.sh valgrind
#	@Scripts/Cleanup.sh	
#	@Scripts/DaemonTest.sh    || exit 903


endif

clean_depend:
	-@rm -f `find -name "*depend"`

unicode:
	@$(MAKE)  all
	@Scripts/Cleanup.sh	
	@$(MAKE)  unicode_scripts

unicode_scripts:
	-@Scripts/RunTest.sh Scripts/AllUnicodeTests.sh
#	-@if [ `cat /proc/sys/kernel/core_pattern` != "%e.core" ]; then echo "WARNING: core name pattern is illegal, please call IT"; fi

rpm:  $(TARGETS_RMX)
	time ./MakeRpm.sh no  | tee /tmp/rpm.txt
	@cat /tmp/rpm.txt | grep "Wrote:" > rpm
	@cat rpm

bin: $(TARGETS_RMX) $(TARGETS_SOFT)
	echo "specify RMX base fodler using RMX_BASE_FOLDER environment var."
	MCMS=$PWD ; cd $(RMX_BASE_FOLDER) ;  ./build.sh --MCMS=$$MCMS | tee /tmp/build.sh.txt
	@cat /tmp/build.sh.txt | grep "version file is ready" > bin
	@cat bin 



#	./MakeRpm.sh "no"

#TAGS: 
#	-@ctags -Re


#MEDIA_MANAGER:
#	echo -n MEDIA_MANAGER > ProductType


