INCPATHLicenseStatus  = -I Libs/LicenseStatus $(COMMONINCPATH)

INCPATHLicenseStatus_RMX  = $(INCPATHLicenseStatus)  $(XML2INC_RMX)
INCPATHLicenseStatus_SOFT  = $(INCPATHLicenseStatus) $(XML2INC_SOFT)

TARGETLicenseStatus_RMX  = $(BIN_RMX)/libLicenseStatus.so
TARGETLicenseStatus_SOFT = $(BIN_SOFT)/libLicenseStatus.so

TARGETS_RMX += $(TARGETLicenseStatus_RMX)
TARGETS_SOFT += $(TARGETLicenseStatus_SOFT)

CPPSOURCESLicenseStatus := $(shell find Libs/LicenseStatus |  grep "\.cpp$$")
SOURCESLicenseStatus :=  $(CPPSOURCESLicenseStatus)
OBJECTSLicenseStatus_RMX := $(CSOURCESLicenseStatus:.c=.$(RMX_PLATFORM).o) $(CPPSOURCESLicenseStatus:.cpp=.$(RMX_PLATFORM).o)
OBJECTSLicenseStatus_SOFT := $(CSOURCESLicenseStatus:.c=.$(SOFT_PLATFORM).o) $(CPPSOURCESLicenseStatus:.cpp=.$(SOFT_PLATFORM).o)

ifndef SKIP_DEPEND
DEPENDSLicenseStatus := $(CSOURCESLicenseStatus:.c=.depend) $(CPPSOURCESLicenseStatus:.cpp=.depend)
DEPENDS += $(DEPENDSLicenseStatus)
-include Libs/LicenseStatus/*depend 
endif

PREREQUIRED_SOURCES += Libs/LicenseStatus/generate.flag

Libs/LicenseStatus/generate.flag: Libs/LicenseStatus/LicenseStatusApi.xml $(XML2SOURCE_BIN)
	@echo Building $@
	$(XML2SOURCE_BIN) Libs/LicenseStatus/LicenseStatusApi.xml Libs/LicenseStatus
	touch $@

Libs/LicenseStatus/%.$(RMX_PLATFORM).o: Libs/LicenseStatus/%.cpp Libs/LicenseStatus/generate.flag
	@echo Building $@
	$(Q)$(DISTCXX_RMX) -c $(CXXFLAGS_RMX) $(INCPATHLicenseStatus_RMX) -o $@ $<

Libs/LicenseStatus/%.$(SOFT_PLATFORM).o: Libs/LicenseStatus/%.cpp Libs/LicenseStatus/generate.flag
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) $(INCPATHLicenseStatus_SOFT) -o $@ $<

ifndef SKIP_DEPEND
Libs/LicenseStatus/%.depend: Libs/LicenseStatus/%.cpp
	@echo Building $@
	$(Q)$(CXX_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_SOFT) $(INCPATHLicenseStatus_SOFT) $<
endif

$(TARGETLicenseStatus_RMX):  $(OBJECTSLicenseStatus_RMX) $(DEPENDSLicenseStatus)
	@echo Building library $@
	$(Q)$(LINK_RMX) $(LFLAGS_RMX) -o $@ $(OBJECTSLicenseStatus_RMX) $(STDLIB_RMX) $(XML2LIB_RMX)  -lpthread -shared

$(TARGETLicenseStatus_SOFT):  $(OBJECTSLicenseStatus_SOFT) $(DEPENDSLicenseStatus)
	@echo Building library $@
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) -o $@ $(OBJECTSLicenseStatus_SOFT) $(STDLIB_SOFT) $(XML2LIB_SOFT) -lpthread -shared
