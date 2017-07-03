

INCPATHIPMCInterface  = -I Processes/IPMCInterface/IPMCInterfaceLib \
					$(COMMONINCPATH)

TARGETIPMCInterface      = $(BIN_RMX)/IPMCInterface
COMMONIPMC =  -I ../../COMMON/IpMc

IPMC_LINKS = Processes/IPMCInterface/IPMCInterfaceLib/IpmcInt.cpp \
		Processes/IPMCInterface/IPMCInterfaceLib/serial.cpp  \
		Processes/IPMCInterface/IPMCInterfaceLib/upgrade_io.cpp  \
		Processes/IPMCInterface/IPMCInterfaceLib/upgrade_serial.cpp 

Processes/IPMCInterface/IPMCInterfaceLib/IpmcInt.cpp:
	$(Q)ln -sf ../../../../../COMMON/IpMc/IpmcInt.cpp $@           

Processes/IPMCInterface/IPMCInterfaceLib/serial.cpp:
	$(Q)ln -sf ../../../../../COMMON/IpMc/serial.cpp $@

Processes/IPMCInterface/IPMCInterfaceLib/upgrade_io.cpp:
	$(Q)ln -sf ../../../../../COMMON/IpMc/upgrade_io.cpp $@           

Processes/IPMCInterface/IPMCInterfaceLib/upgrade_serial.cpp:
	$(Q)ln -sf ../../../../../COMMON/IpMc/upgrade_serial.cpp $@           



CSOURCESIPMCInterface := $(shell find Processes/IPMCInterface/IPMCInterfaceLib | grep -v /Tests/ | grep "\.c$$" | grep -v upgradefw)
CPPSOURCESIPMCInterface := $(shell find Processes/IPMCInterface/IPMCInterfaceLib -type f |  grep -v /Tests/ | grep "\.cpp$$" | grep -v upgradefw) $(IPMC_LINKS)
SOURCESIPMCInterface := $(CSOURCESIPMCInterface) $(CPPSOURCESIPMCInterface)
OBJECTSIPMCInterface := $(CSOURCESIPMCInterface:.c=.o) $(CPPSOURCESIPMCInterface:.cpp=.o)
ifndef SKIP_DEPEND
DEPENDSIPMCInterface := $(CSOURCESIPMCInterface:.c=.depend) $(CPPSOURCESIPMCInterface:.cpp=.depend)
endif

TARGETS_RMX += $(TARGETIPMCInterface)

ifndef SKIP_DEPEND
-include $(DEPENDSIPMCInterface)
endif

Processes/IPMCInterface/IPMCInterfaceLib/%.o: Processes/IPMCInterface/IPMCInterfaceLib/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX_RMX) -c $(CXXFLAGS) -Wno-return-type -O0 $(INCPATHIPMCInterface) $(COMMONIPMC) -o $@ $<

Processes/IPMCInterface/IPMCInterfaceLib/%.o: Processes/IPMCInterface/IPMCInterfaceLib/%.c
	@echo Building $@
	$(Q)$(DISTCC_RMX) -c $(CFLAGS_RMX) -Wno-return-type -O0 $(INCPATHIPMCInterface) $(COMMONIPMC) -o $@ $<

ifndef SKIP_DEPEND
Processes/IPMCInterface/IPMCInterfaceLib/%.depend: Processes/IPMCInterface/IPMCInterfaceLib/%.cpp
	@echo Building $@
	$(Q)$(CXX_RMX) -MM -MF $(patsubst %.o,%.depend,$@) -MT $@ $(CXXFLAGS_RMX) $(INCPATHIPMCInterface) $(COMMONIPMC) $<

Processes/IPMCInterface/IPMCInterfaceLib/%.depend: Processes/IPMCInterface/IPMCInterfaceLib/%.c
	@echo Building $@
	$(Q)$(CC_RMX) -MM -MF $(patsubst %.o,%.depend,$@) -MT $@ $(CXXFLAGS_RMX) $(INCPATHIPMCInterface) $(COMMONIPMC) $<
endif


$(TARGETIPMCInterface):  $(OBJECTSIPMCInterface) $(BASE_DEPEND_RMX) $(MAINOBJ_RMX)
	@echo Building $@
	$(Q)$(LINK_RMX) $(LFLAGS_RMX) $(OBJECTSIPMCInterface) $(BASE_LIBS_RMX) $(MAINOBJ_RMX) -o $@




