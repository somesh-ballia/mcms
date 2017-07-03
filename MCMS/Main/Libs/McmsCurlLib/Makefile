INCPATHMcmsCurlLib  = \
	-I Libs/McmsCurlLib \
	-I Libs/ProcessBase \
	-I Libs/SimLinux \
	-I IncludeInternalMcms \
	-I Libs/Common \
	-I ../McmIncld/Common \
	-I Libs/XmlPars \
	$(XML2INC)

INCPATHMcmsCurlLib_RMX = $(INCPATHMcmsCurlLib) $(ENCRYPTINC_RMX) $(CURLINC_RMX)
INCPATHMcmsCurlLib_SOFT = $(INCPATHMcmsCurlLib) $(ENCRYPTINC_SOFT) $(CURLINC_SOFT)

TARGETMcmsCurlLib_RMX = $(BIN_RMX)/libMcmsCurlLib.so
TARGETMcmsCurlLib_SOFT = $(BIN_SOFT)/libMcmsCurlLib.so

CSOURCESMcmsCurlLib := $(shell find Libs/McmsCurlLib | grep -v /Tests/ | grep "\.c$$")
CPPSOURCESMcmsCurlLib := $(shell find Libs/McmsCurlLib |  grep -v /Tests/ | grep "\.cpp$$")
SOURCESMcmsCurlLib := $(CSOURCESMcmsCurlLib) $(CPPSOURCESMcmsCurlLib)
OBJECTSMcmsCurlLib_RMX := $(CSOURCESMcmsCurlLib:.c=.$(RMX_PLATFORM).o) $(CPPSOURCESMcmsCurlLib:.cpp=.$(RMX_PLATFORM).o)
OBJECTSMcmsCurlLib_SOFT := $(CSOURCESMcmsCurlLib:.c=.$(SOFT_PLATFORM).o) $(CPPSOURCESMcmsCurlLib:.cpp=.$(SOFT_PLATFORM).o)

ifndef SKIP_DEPEND
DEPENDSMcmsCurlLib := $(CSOURCESMcmsCurlLib:.c=.depend) $(CPPSOURCESMcmsCurlLib:.cpp=.depend)
endif

TARGETS_RMX += $(TARGETMcmsCurlLib_RMX) 
TARGETS_SOFT += $(TARGETMcmsCurlLib_SOFT) 

ifndef SKIP_DEPEND
DEPENDS += $(DEPENDSMcmsCurlLib)
-include Libs/McmsCurlLib/*depend
endif

Libs/McmsCurlLib/%.$(RMX_PLATFORM).o: Libs/McmsCurlLib/%.cpp 
	@echo Building $@
	$(Q)$(DISTCXX_RMX) -c $(CXXFLAGS_RMX) $(INCPATHMcmsCurlLib_RMX) -o $@ $<

Libs/McmsCurlLib/%.$(RMX_PLATFORM).o: Libs/McmsCurlLib/%.c 
	@echo Building $@
	$(Q)$(DISTCC_RMX) -c $(CFLAGS_RMX) $(INCPATHMcmsCurlLib_RMX) -o $@ $<

Libs/McmsCurlLib/%.$(SOFT_PLATFORM).o: Libs/McmsCurlLib/%.cpp 
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) $(INCPATHMcmsCurlLib_SOFT) -o $@ $<

Libs/McmsCurlLib/%.$(SOFT_PLATFORM).o: Libs/McmsCurlLib/%.c 
	@echo Building $@
	$(Q)$(DISTCC_SOFT) -c $(CFLAGS_SOFT) $(INCPATHMcmsCurlLib_SOFT) -o $@ $<

ifndef SKIP_DEPEND
Libs/McmsCurlLib/%.depend: Libs/McmsCurlLib/%.cpp
	$(Q)$(CXX_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_SOFT) $(INCPATHMcmsCurlLib_SOFT) $<

Libs/McmsCurlLib/%.depend: Libs/McmsCurlLib/%.c
	$(Q)$(CC_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(FLAGS_SOFT) $(INCPATHMcmsCurlLib_SOFT) $<
endif

$(TARGETMcmsCurlLib_RMX):  $(OBJECTSMcmsCurlLib_RMX) $(DEPENDSMcmsCurlLib)
	@echo Building library $@
	$(Q)$(LINK_RMX) $(LFLAGS_RMX) -o $@ $(OBJECTSMcmsCurlLib_RMX) $(STDLIB_RMX) $(CURLLIB_RMX) $(XML2LIB) -lpthread -shared

$(TARGETMcmsCurlLib_SOFT):  $(OBJECTSMcmsCurlLib_SOFT) $(DEPENDSMcmsCurlLib)
	@echo Building library $@
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) -o $@ $(OBJECTSMcmsCurlLib_SOFT) $(STDLIB_SOFT) $(CURLLIB_SOFT) $(XML2LIB) -lpthread -shared
