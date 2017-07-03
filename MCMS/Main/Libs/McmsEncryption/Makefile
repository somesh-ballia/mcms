INCPATHMcmsEncryption  = \
	-I Libs/McmsEncryption \
	-I Libs/ProcessBase \
	-I Libs/SimLinux \
	-I IncludeInternalMcms \
	-I Libs/Common \
	-I ../McmIncld/Common \
	-I Libs/XmlPars \
	$(XML2INC)

INCPATHMcmsEncryption_RMX = $(INCPATHMcmsEncryption) $(ENCRYPTINC_RMX)
INCPATHMcmsEncryption_SOFT = $(INCPATHMcmsEncryption) $(ENCRYPTINC_SOFT)

TARGETMcmsEncryption_RMX = $(BIN_RMX)/libMcmsEncryption.so
TARGETMcmsEncryption_SOFT = $(BIN_SOFT)/libMcmsEncryption.so

CSOURCESMcmsEncryption := $(shell find Libs/McmsEncryption | grep -v /Tests/ | grep "\.c$$")
CPPSOURCESMcmsEncryption := $(shell find Libs/McmsEncryption |  grep -v /Tests/ | grep "\.cpp$$")
SOURCESMcmsEncryption := $(CSOURCESMcmsEncryption) $(CPPSOURCESMcmsEncryption)
OBJECTSMcmsEncryption_RMX := $(CSOURCESMcmsEncryption:.c=.$(RMX_PLATFORM).o) $(CPPSOURCESMcmsEncryption:.cpp=.$(RMX_PLATFORM).o)
OBJECTSMcmsEncryption_SOFT := $(CSOURCESMcmsEncryption:.c=.$(SOFT_PLATFORM).o) $(CPPSOURCESMcmsEncryption:.cpp=.$(SOFT_PLATFORM).o)

ifndef SKIP_DEPEND
DEPENDSMcmsEncryption := $(CSOURCESMcmsEncryption:.c=.depend) $(CPPSOURCESMcmsEncryption:.cpp=.depend)
endif

TARGETS_RMX += $(TARGETMcmsEncryption_RMX) 
TARGETS_SOFT += $(TARGETMcmsEncryption_SOFT) 

ifndef SKIP_DEPEND
DEPENDS += $(DEPENDSMcmsEncryption)

-include Libs/McmsEncryption/*depend
endif

Libs/McmsEncryption/%.$(RMX_PLATFORM).o: Libs/McmsEncryption/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX_RMX) -c $(CXXFLAGS_RMX) $(INCPATHMcmsEncryption_RMX) -o $@ $<

Libs/McmsEncryption/%.$(RMX_PLATFORM).o: Libs/McmsEncryption/%.c
	@echo Building $@
	$(Q)$(DISTCC_RMX) -c $(CFLAGS_RMX) $(INCPATHMcmsEncryption_RMX) -o $@ $<

Libs/McmsEncryption/%.$(SOFT_PLATFORM).o: Libs/McmsEncryption/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) $(INCPATHMcmsEncryption_SOFT) -o $@ $<

Libs/McmsEncryption/%.$(SOFT_PLATFORM).o: Libs/McmsEncryption/%.c
	@echo Building $@
	$(Q)$(DISTCC_SOFT) -c $(CFLAGS_SOFT) $(INCPATHMcmsEncryption_SOFT) -o $@ $<

ifndef SKIP_DEPEND
Libs/McmsEncryption/%.depend: Libs/McmsEncryption/%.cpp
	$(Q)$(CXX_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_SOFT) $(INCPATHMcmsEncryption_SOFT) $<

Libs/McmsEncryption/%.depend: Libs/McmsEncryption/%.c
	$(Q)$(CC_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(FLAGS_SOFT) $(INCPATHMcmsEncryption_SOFT) $<
endif

$(TARGETMcmsEncryption_RMX):  $(OBJECTSMcmsEncryption_RMX)  $(DEPENDSMcmsEncryption)
	@echo Building library $@
	$(Q)$(LINK_RMX) $(LFLAGS_RMX) -o $@ $(OBJECTSMcmsEncryption_RMX) $(STDLIB_RMX) $(XML2LIB) -lpthread -shared

$(TARGETMcmsEncryption_SOFT):  $(OBJECTSMcmsEncryption_SOFT)  $(DEPENDSMcmsEncryption)
	@echo Building library $@
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) -o $@ $(OBJECTSMcmsEncryption_SOFT) $(STDLIB_SOFT) $(XML2LIB) -lpthread -shared
