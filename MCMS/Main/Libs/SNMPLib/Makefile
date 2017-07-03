INCPATHSNMPLib  = \
	-I Libs/SNMPLib \
	-I Libs/ProcessBase \
	-I Libs/XmlPars \
	-I Libs/Common \
	-I ../McmIncld/CS/CsSignaling \
	-I ../McmIncld/MPL/Card/CardCommon \
	-I IncludeInternalMcms \
	-I ../McmIncld/Common \
	-I ../McmIncld/MPL/Card/CardMngrMaintenance \
	-I ../McmIncld/MPL/Card/CardMngrRecording \
	-I ../McmIncld/MPL/Card/CardMngrTB \
	-I ../McmIncld/MPL/Card/CardMngrIvrCntl \
	-I ../McmIncld/MPL/Card/PhysicalPortART/Audio \
	-I ../McmIncld/MPL/Card/PhysicalPortAudioCntl \
	-I ../McmIncld/MPL/Card/PhysicalPortVideo \
	-I ../McmIncld/MPL/Card/CardMngrIpMedia \
	-I ../McmIncld/MPL/ShelfMngr \
	$(XML2INC)

INCPATHSNMPLib_RMX = $(INCPATHSNMPLib) $(SNMPINC_RMX)
INCPATHSNMPLib_SOFT = $(INCPATHSNMPLib) $(SNMPINC_SOFT)

TARGETSNMPLib_RMX = $(BIN_RMX)/libSNMP.so
TARGETSNMPLib_SOFT = $(BIN_SOFT)/libSNMP.so

TARGETSNMP_RMX=$(BIN_RMX)/snmpd
TARGETSNMPJ_RMX=$(BIN_RMX)/snmpdj
TARGETSNMP_SOFT=$(BIN_SOFT)/snmpd
TARGETSNMPJ_SOFT=$(BIN_SOFT)/snmpdj

SOURCESSNMPLib := $(shell find Libs/SNMPLib |  grep -v /Tests/ | grep "\.cpp$$")
OBJECTSSNMPLib_RMX := $(SOURCESSNMPLib:.cpp=.$(RMX_PLATFORM).o)
OBJECTSSNMPLib_SOFT := $(SOURCESSNMPLib:.cpp=.$(SOFT_PLATFORM).o)

ifndef SKIP_DEPEND
DEPENDSSNMPLib := $(SOURCESSNMPLib:.cpp=.depend)
endif

TARGETS_RMX += $(TARGETSNMPLib_RMX) $(TARGETSNMP_RMX) $(TARGETSNMPJ_RMX)
TARGETS_SOFT += $(TARGETSNMPLib_SOFT) $(TARGETSNMP_SOFT) $(TARGETSNMPJ_SOFT)

ifndef SKIP_DEPEND
DEPENDS += $(DEPENDSSNMPLib)

-include Libs/SNMPLib/*depend
endif

Libs/SNMPLib/%.$(RMX_PLATFORM).o: Libs/SNMPLib/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX_RMX) -c $(CXXFLAGS_RMX) $(INCPATHSNMPLib_RMX) -o $@ $<

Libs/SNMPLib/%.$(SOFT_PLATFORM).o: Libs/SNMPLib/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) $(INCPATHSNMPLib_SOFT) -o $@ $<

ifndef SKIP_DEPEND
Libs/SNMPLib/%.depend: Libs/SNMPLib/%.cpp
	$(Q)$(CXX_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_SOFT) $(INCPATHSNMPLib_SOFT) $< 
endif

$(TARGETSNMPLib_RMX):  $(OBJECTSSNMPLib_RMX)  $(DEPENDSSNMPLib)  $(DEPENDSSNMPLib)
	@echo Building library $@
	$(Q)$(LINK_RMX) $(LFLAGS_RMX) -o $@ $(OBJECTSSNMPLib_RMX) -lpthread -shared 

$(TARGETSNMPLib_SOFT):  $(OBJECTSSNMPLib_SOFT)  $(DEPENDSSNMPLib)  $(DEPENDSSNMPLib)
	@echo Building library $@
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) -o $@ $(OBJECTSSNMPLib_SOFT) -lpthread -shared 


$(TARGETSNMP_RMX):
	@echo Building $@
	$(Q)(echo "#!/bin/sh") >  $@
	$(Q)(echo "rm -f /tmp/netsnmpfipsflag") >> $@
	$(Q)(echo "./snmpd  -Lf /tmp/snmpd.log -C -c /mcms/Cfg/snmpd.conf,/tmp/snmpd.conf") >> $@
	$(Q)chmod a+x $@

$(TARGETSNMP_SOFT):
	@echo Building $@
	$(Q)(echo "#!/bin/sh") >  $@
	$(Q)(echo "rm -f /tmp/netsnmpfipsflag") >> $@
	$(Q)(echo "sudo /bin/env LD_LIBRARY_PATH=/mcms/Bin /mcms/snmpd  -Lf /tmp/snmpd.log -C -c /mcms/Cfg/snmpd.conf,/tmp/snmpd.conf") >> $@
	$(Q)chmod a+x $@


$(TARGETSNMPJ_RMX):
	@echo Building $@
	$(Q)(echo "#!/bin/sh") >  $@
	$(Q)(echo "touch /tmp/netsnmpfipsflag") >> $@  # snmpd is patched, -F means run in FIPS mode
	$(Q)(echo "./snmpd -F  -Lf /tmp/snmpd.log -C -c /mcms/Cfg/snmpd.conf,/tmp/snmpd.conf") >> $@
	$(Q)chmod a+x $@

$(TARGETSNMPJ_SOFT):
	@echo Building $@
	$(Q)(echo "#!/bin/sh") >  $@
	$(Q)(echo "touch /tmp/netsnmpfipsflag") >> $@  # snmpd is patched, -F means run in FIPS mode
	$(Q)(echo "sudo /bin/env LD_LIBRARY_PATH=/mcms/Bin /mcms/snmpd -F  -Lf /tmp/snmpd.log -C -c /mcms/Cfg/snmpd.conf,/tmp/snmpd.conf") >> $@
	$(Q)chmod a+x $@


