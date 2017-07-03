INCPATHCommon = -I Libs/Common \
	-I Libs/XmlPars \
	-I Libs/ProcessBase \
	-I Libs/SimLinux \
	-I Libs/BFCPLib \
	-I IncludeInternalMcms \
	-I ../McmIncld/CS/CsSignaling \
	-I ../McmIncld/CS/CsMaintenance \
	-I ../McmIncld/Common \
	-I ../McmIncld/MPL/Card/CardCommon \
	-I ../McmIncld/MPL/Card/CardMngrIpMedia/ \
	-I ../McmIncld/MPL/Card/PhysicalPortART/RTP \
	-I ../McmIncld/MPL/Card/CardMngrTB \
	-I ../McmIncld/MPL/Card/PhysicalPortVideo \
	-I ../McmIncld/MPL/Card/PhysicalPortAudioCntl \
	-I ../McmIncld/MPL/Card/PhysicalPortART/Audio \
	-I ../McmIncld/MPL/Card/PhysicalPortART/Common \
	-I ../McmIncld/MPL/Card/PhysicalPortART/MUX \
	-I ../McmIncld/MPL/Card/CardMngrIvrCntl \
	-I ../McmIncld/MPL/ShelfMngr \
	-I ../McmIncld/MPL/Card/CardMngrRecording \
	-I ../McmIncld/MPL/Card/PhisycalPortNetISDN \
	-I ../McmIncld/MPL/Card/CardMngrMaintenance \
	-I ../McmIncld/MPL/Card/RtmIsdnMaintenance \
	-I ../McmIncld/MPL/Card/CardMngrICE \
	-I ../McmIncld/MPL/Card/CardMngrTIP \
	-I ../McmIncld/MPL/Card/PhysicalMrmp \
	-I ../McmIncld/MPL/Card/CardMngrBFCP  \
        -I ./boost_1_53_0  \
	-I ../McmIncld/MPL/Shared/802_1x \
	 $(XML2INC)
	

INCPATHCommon_RMX  = $(INCPATHCommon) $(ZLIBINC_RMX) $(CPPUNITINC_RMX)
INCPATHCommon_SOFT  = $(INCPATHCommon) $(ZLIBINC_SOFT) $(CPPUNITINC_SOFT)

ifndef SKIP_TESTS
INCPATHCommonLibTest  =  $(INCPATHCommon_SOFT) \
					    -I Libs/Common/Tests \
					    $(CPPUNITINC)
endif

TARGETCommon_RMX  = $(BIN_RMX)/libCommon.so
TARGETCommon_SOFT = $(BIN_SOFT)/libCommon.so

ifndef SKIP_TESTS
TARGETCommonLibTest = $(BIN_SOFT)/CommonLib.Test
endif

TARGETS_RMX += $(TARGETCommon_RMX)
TARGETS_SOFT += $(TARGETCommon_SOFT)
ifndef SKIP_TESTS
TEST_TARGETS += $(TARGETCommonLibTest)
endif

CSOURCESCommon := $(shell find Libs/Common | grep -v /Tests/ | grep "\.c$$")
CPPSOURCESCommon := $(shell find Libs/Common |  grep -v /Tests/ | grep "\.cpp$$")
SOURCESCommon := $(CSOURCESCommon) $(CPPSOURCESCommon)
OBJECTSCommon_RMX := $(CSOURCESCommon:.c=.$(RMX_PLATFORM).o) $(CPPSOURCESCommon:.cpp=.$(RMX_PLATFORM).o)
OBJECTSCommon_SOFT := $(CSOURCESCommon:.c=.$(SOFT_PLATFORM).o) $(CPPSOURCESCommon:.cpp=.$(SOFT_PLATFORM).o)

ifndef SKIP_TESTS
SOURCESCommonLibTest := $(shell find Libs/Common/Tests | grep "\.cpp$$")
OBJECTSCommonLibTest := $(SOURCESCommonLibTest:.cpp=.o)
endif

ifndef SKIP_DEPEND
	DEPENDSCommon := $(CSOURCESCommon:.c=.depend) $(CPPSOURCESCommon:.cpp=.depend)
	DEPENDSCommonLibTest := $(SOURCESCommonLibTest:.cpp=.depend)
	DEPENDS += $(DEPENDSCommonLibTest) $(DEPENDSCommon)
	-include Libs/Common/*depend
	-include Libs/Common/Tests/*depend
endif

Libs/Common/%.$(RMX_PLATFORM).o: Libs/Common/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX_RMX) -c $(CXXFLAGS_RMX) $(INCPATHCommon_RMX) -o $@ $<

Libs/Common/%.$(RMX_PLATFORM).o: Libs/Common/%.c
	@echo Building $@
	$(Q)$(DISTCC_RMX) -c $(CFLAGS_RMX) $(INCPATHCommon_RMX) -o $@ $<

Libs/Common/%.$(SOFT_PLATFORM).o: Libs/Common/%.cpp 
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) $(INCPATHCommon_SOFT) -o $@ $<

Libs/Common/%.$(SOFT_PLATFORM).o: Libs/Common/%.c
	@echo Building $@
	$(Q)$(DISTCC_SOFT) -c $(CFLAGS_SOFT) $(INCPATHCommon_SOFT) -o $@ $<

# <<<< Added for suppressing the -Wformat warning in ObjString.cpp	
Libs/Common/ObjString.$(RMX_PLATFORM).o: Libs/Common/ObjString.cpp
	@echo Building $@
	$(Q)$(DISTCXX_RMX) -c $(CXXFLAGS_RMX) -Wno-format-nonliteral $(INCPATHCommon_RMX) -o $@ $<

Libs/Common/ObjString.$(SOFT_PLATFORM).o: Libs/Common/ObjString.cpp 
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) -Wno-format-nonliteral $(INCPATHCommon_SOFT) -o $@ $<
# >>>> Added for suppressing the -Wformat warning in ObjString.cpp

# <<<< Added for suppressing the -Wformat warning in SipUtils.cpp	
Libs/Common/SipUtils.$(RMX_PLATFORM).o: Libs/Common/SipUtils.cpp
	@echo Building $@
	$(Q)$(DISTCXX_RMX) -c $(CXXFLAGS_RMX) -Wno-format-nonliteral $(INCPATHCommon_RMX) -o $@ $<

Libs/Common/SipUtils.$(SOFT_PLATFORM).o: Libs/Common/SipUtils.cpp 
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) -Wno-format-nonliteral $(INCPATHCommon_SOFT) -o $@ $<
# >>>> Added for suppressing the -Wformat warning in SipUtils.cpp


ifndef SKIP_DEPEND
Libs/Common/%.depend: Libs/Common/%.cpp
	@echo Building $@
	$(Q)$(CXX_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_SOFT) $(INCPATHCommon_SOFT) $<

Libs/Common/%.depend: Libs/Common/%.c 
	@echo Building $@
	$(Q)$(CC_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CFLAGS_SOFT) $(INCPATHCommon_SOFT) $<
endif

$(TARGETCommon_RMX):  $(OBJECTSCommon_RMX) $(DEPENDSCommon)
	@echo Building library $@
	$(Q)$(LINK_RMX) $(LFLAGS_RMX) -o $@ $(OBJECTSCommon_RMX) $(STDLIB_RMX) $(XML2LIB) -lpthread -shared

$(TARGETCommon_SOFT):  $(OBJECTSCommon_SOFT) $(DEPENDSCommon)
	@echo Building library $@
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) -o $@ $(OBJECTSCommon_SOFT) $(STDLIB_SOFT) $(XML2LIB)  -lpthread -shared


ifndef SKIP_TESTS

Libs/Common/Tests/%.o: Libs/Common/Tests/%.cpp
	@echo Building $@
	$(Q)$(DISTCC_SOFT) -c $(CXXFLAGS_SOFT) $(INCPATHCommonLibTest) -o $@ $<

ifndef SKIP_DEPEND
Libs/Common/Tests/%.depend: Libs/Common/Tests/%.cpp
	@echo Building $@
	$(Q)$(CXX_SOFT) -MM -MF $(patsubst %.o,%.depend,$@) -MT $@ $(CXXFLAGS) $(INCPATHCommonLibTest) $<
endif

$(TARGETCommonLibTest): $(OBJECTSCommonLibTest) $(DEPENDSCommon)  $(DEPENDSCommonLibTest) $(BASE_DEPEND_SOFT)
	@echo Building $(TARGETCommonLibTest)
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) $(OBJECTSCommonLibTest) $(CPPUNITLIB_SOFT) -lz -lrt -LBin $(DYN_LIBS) $(BASE_LIBS_SOFT) -o $@
endif
