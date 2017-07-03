
INCPATHApacheModule  = -I Processes/ApacheModule/ApacheModuleLib \
					   -I Processes/ApacheModule \
					$(COMMONINCPATH) $(HTTPINC_RMX) $(ZLIBINC_RMX) $(ENCRYPTINC_RMX) \
					   -D_CONSOLE

TARGETApacheModule_RMX      = $(BIN_RMX)/ApacheModule
TARGETApacheModule_SOFT      = $(BIN_SOFT)/ApacheModule


CSOURCESApacheModule := $(shell find Processes/ApacheModule/ApacheModuleLib | grep -v /Tests/ | grep "\.c$$")
CPPSOURCESApacheModule := $(shell find Processes/ApacheModule/ApacheModuleLib |  grep -v /Tests/ | grep "\.cpp$$")
SOURCESApacheModule := $(CSOURCESApacheModule) $(CPPSOURCESApacheModule)
OBJECTSApacheModule := $(CSOURCESApacheModule:.c=.o) $(CPPSOURCESApacheModule:.cpp=.o)
ifndef SKIP_DEPEND
DEPENDSApacheModule := $(CSOURCESApacheModule:.c=.depend) $(CPPSOURCESApacheModule:.cpp=.depend)
endif

TARGETS_RMX += $(TARGETApacheModule_RMX)
TARGETS_SOFT += $(TARGETApacheModule_SOFT)

ifndef SKIP_DEPEND
-include $(DEPENDSApacheModule)
endif


Processes/ApacheModule/%.o: Processes/ApacheModule/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX) -c $(CXXFLAGS) $(INCPATHApacheModule) -o $@ $<

Processes/ApacheModule/%.o: Processes/ApacheModule/%.c 
	@echo Building $@
	$(Q)$(DISTCC) -c $(CFLAGS) $(INCPATHApacheModule) -o $@ $<

Processes/ApacheModule/ApacheModuleLib/%.o: Processes/ApacheModule/ApacheModuleLib/%.cpp 
	@echo Building $@
	$(Q)$(DISTCXX) -c $(CXXFLAGS) $(INCPATHApacheModule) -o $@ $<

Processes/ApacheModule/ApacheModuleLib/%.o: Processes/ApacheModule/ApacheModuleLib/%.c 
	@echo Building $@
	$(Q)$(DISTCC) -c $(CFLAGS) $(INCPATHApacheModule) -o $@ $<

ifndef SKIP_DEPEND
Processes/ApacheModule/%.depend: Processes/ApacheModule/%.cpp
	@echo Building $@
	$(Q)$(CXX) -MM -MF $(patsubst %.o,%.depend,$@) -MT $@ $(CXXFLAGS) $(INCPATHApacheModule) $<


Processes/ApacheModule/%.depend: Processes/ApacheModule/%.c 
	@echo Building $@
	$(Q)$(CC) -MM -MF $(patsubst %.o,%.depend,$@) -MT $@ $(CFLAGS) $(INCPATHApacheModule) $<


Processes/ApacheModule/ApacheModuleLib/%.depend: Processes/ApacheModule/ApacheModuleLib/%.cpp 
	@echo Building $@
	$(Q)$(CXX) -MM -MF $(patsubst %.o,%.depend,$@) -MT $@ $(CXXFLAGS) $(INCPATHApacheModule) $<


Processes/ApacheModule/ApacheModuleLib/%.depend: Processes/ApacheModule/ApacheModuleLib/%.c 
	@echo Building $@
	$(Q)$(CC) -MM -MF $(patsubst %.o,%.depend,$@) -MT $@ $(CFLAGS) $(INCPATHApacheModule) $<
endif


$(BIN_RMX)/mod_polycom.so: $(OBJECTSApacheModule) $(BASE_DEPEND_RMX) $(BIN_RMX)/libMcmsEncryption.so Processes/ApacheModule/ModuleMain.o
	@echo Building $@
	$(Q)$(LINK) $(LFLAGS) $(OBJECTSApacheModule) Processes/ApacheModule/ModuleMain.o  $(BASE_LIBS_RMX) $(ENCRYPTLIB_RMX) -shared -o $@

$(BIN_SOFT)/mod_polycom.so: $(OBJECTSApacheModule) $(BASE_DEPEND_SOFT) $(BIN_SOFT)/libMcmsEncryption.so Processes/ApacheModule/ModuleMain.o
	@echo Building $@
	$(Q)$(LINK) $(LFLAGS) $(OBJECTSApacheModule) Processes/ApacheModule/ModuleMain.o  $(BASE_LIBS_SOFT) $(ENCRYPTLIB_SOFT) -shared -o $@


#temp solution
#$(BIN_SOFT)/mod_polycom.so: $(BIN_RMX)/mod_polycom.so
#	@echo Building $@
#	@echo "warning: mod_polycom.so for soft mcu is taken from RMX binaries"
#	$(Q)cp $(BIN_RMX)/mod_polycom.so $(BIN_SOFT)/mod_polycom.so


$(TARGETApacheModule_RMX): $(BIN_RMX)/mod_polycom.so
	@echo Building $@
	$(Q)echo "#!/bin/sh" > $@
	$(Q)echo "" >> $@
	$(Q)echo "CMD='sudo env MCU_HOME_DIR= LD_LIBRARY_PATH=/usr/local/apache2/lib:/mcms/Bin /mcms/Bin/httpd'" >> $@
	$(Q)echo "CFG=/mcms/StaticCfg/httpd.conf.sim" >> $@
	$(Q)echo "LOG=/tmp/startup_logs/DaemonProcessesLoad.log" >> $@
	$(Q)echo "" >> $@
	$(Q)echo "sleep 1" >> $@
	$(Q)echo "\$$CMD -f \$$CFG 2>&1 | tee -a \$$LOG" >> $@
	$(Q)echo "while true; do" >> $@
	$(Q)echo "  sleep 10" >> $@
	$(Q)echo "  ps -e | grep -v grep | grep httpd >/dev/null || \$$CMD -f \$$CFG 2>&1 | tee -a \$$LOG" >> $@
	$(Q)echo "done" >> $@
	$(Q)chmod +x $@

$(TARGETApacheModule_SOFT): $(TARGETApacheModule_RMX) $(BIN_SOFT)/mod_polycom.so
	@echo Building $@
	$(Q)cp $(TARGETApacheModule_RMX) $(TARGETApacheModule_SOFT)

