INCPATHCsModule  = \
	-I Processes/CsModule/CsModuleLib \
	-I Processes/CsModule \
	$(COMMONINCPATH)

TARGETCsModule_RMX  = $(BIN_RMX)/libMcmsCs.so
TARGETCsModule_SOFT = $(BIN_SOFT)/libMcmsCs.so

CPPSOURCESCsModule := Processes/CsModule/CSInterface.cpp
SOURCESCsModule := $(CPPSOURCESCsModule)
OBJECTSCsModule := $(CPPSOURCESCsModule:.cpp=.o)

ifndef SKIP_DEPEND
DEPENDSCsModule := $(CPPSOURCESCsModule:.cpp=.depend)
endif

TARGETS_RMX += $(TARGETCsModule_RMX)
TARGETS_SOFT += $(TARGETCsModule_SOFT)

ifndef SKIP_DEPEND
-include $(DEPENDSCsModule)
endif

Processes/CsModule/%.o: Processes/CsModule/%.cpp 
	@echo Building $@
	$(Q)$(DISTCXX_RMX) -c $(CXXFLAGS_RMX) $(INCPATHCsModule) -o $@ $<

ifndef SKIP_DEPEND
Processes/CsModule/%.depend: Processes/CsModule/%.cpp 
	@echo Building $@
	$(Q)$(CXX_RMX) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_RMX) $(INCPATHCsModule) $<
endif

$(TARGETCsModule_RMX): $(OBJECTSCsModule) $(BASE_DEPEND_RMX)
	@echo Building $@
	$(Q)$(LINK_RMX) $(LFLAGS_RMX) $(OBJECTSCsModule)  $(BASE_LIBS_RMX)  -shared -o $@

#temp solution
$(TARGETCsModule_SOFT): $(TARGETCsModule_RMX)
	@echo Building $@
	@echo "warning: libMcmsCs.so for soft mcu is taken from RMX binaries"
	$(Q)cp $(TARGETCsModule_RMX) $(TARGETCsModule_SOFT)
