INCPATHDiagnostics  = \
	-I Utils/Diagnostics \
	$(COMMONINCPATH) 

TARGETDiagnostics   = $(BIN_RMX)/Diagnostics

CSOURCESDiagnostics := $(shell find Utils/Diagnostics | grep -v /Tests/ | grep "\.c$$")
CPPSOURCESDiagnostics := $(shell find Utils/Diagnostics |  grep -v /Tests/ | grep "\.cpp$$")
SOURCESDiagnostics := $(CSOURCESDiagnostics) $(CPPSOURCESDiagnostics)
OBJECTSDiagnostics := $(CSOURCESDiagnostics:.c=.o) $(CPPSOURCESDiagnostics:.cpp=.o)

ifndef SKIP_DEPEND
DEPENDSDiagnostics := $(CSOURCESDiagnostics:.c=.depend) $(CPPSOURCESDiagnostics:.cpp=.depend)
endif

TARGETS_RMX += $(TARGETDiagnostics)

ifndef SKIP_DEPEND
DEPENDS += $(DEPENDSDiagnostics)
-include Utils/Diagnostics/*depend
endif

#-include $(DEPENDSDiagnostics)

Utils/Diagnostics/%.o: Utils/Diagnostics/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX_RMX) -c $(CXXFLAGS_RMX) $(INCPATHDiagnostics) -o $@ $<

Utils/Diagnostics/%.o: Utils/Diagnostics/%.c
	@echo Building $@
	$(Q)$(DISTCC_RMX) -c $(CFLAGS_RMX) $(INCPATHDiagnostics) -o $@ $<

ifndef SKIP_DEPEND
Utils/Diagnostics/%.depend: Utils/Diagnostics/%.cpp
	@echo Building $@
	$(Q)$(CXX_RMX) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_RMX) $(INCPATHDiagnostics) $<

Utils/Diagnostics/%.depend: Utils/Diagnostics/%.c
	@echo Building $@
	$(Q)$(CC_RMX) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_RMX) $(INCPATHDiagnostics) $<
endif

$(TARGETDiagnostics):  $(OBJECTSDiagnostics)   $(DEPENDSDiagnostics)
	@echo Building $@
	$(Q)$(LINK_RMX) $(LFLAGS_RMX) $(OBJECTSDiagnostics)  $(LINK_FLAGS_RMX) -lpthread -o $@
