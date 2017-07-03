INCPATHLedLight  = -DLED_SRV -I Utils/LedLight \
                             -I Utils/LedLight/ledApi \
                                   $(COMMONINCPATH)


TARGETLedLightSrv   = $(BIN_SOFT)/LightSrv

CSOURCESLedLightSrv := $(shell find Utils/LedLight |  grep -v Client | grep -v Utils/LedLight/ledApi | grep "\.c$$")
CPPSOURCESLedLightSrv :=  $(shell find Utils/LedLight |  grep -v Client | grep -v Utils/LedLight/ledApi | grep "\.cpp$$")
SOURCESLedLightSrv := $(CSOURCESLedLightSrv) $(CPPSOURCESLedLightSrv)
OBJECTSLedLightSrv := $(CSOURCESLedLightSrv:.c=.o) $(CPPSOURCESLedLightSrv:.cpp=.o)
DEPENDSLedLightSrv := $(CSOURCESLedLightSrv:.c=.depend) $(CPPSOURCESLedLightSrv:.cpp=.depend)
                                
TARGETLedLightCli   = $(BIN_SOFT)/LightCli

CSOURCESLedLightCli := 
CPPSOURCESLedLightCli := Utils/LedLight/main.cpp Utils/LedLight/LedLightClient.cpp Utils/LedLight/LedLightTask.cpp Utils/LedLight/BaseProcessor.cpp
SOURCESLedLightCli := $(CSOURCESLedLightCli) $(CPPSOURCESLedLightCli)
OBJECTSLedLightCli := $(CSOURCESLedLightCli:.c=.o) $(CPPSOURCESLedLightCli:.cpp=.o)
DEPENDSLedLightCli := $(CSOURCESLedLightCli:.c=.depend) $(CPPSOURCESLedLightCli:.cpp=.depend)

TARGETS_SOFT += $(TARGETLedLightSrv)
TARGETS_SOFT += $(TARGETLedLightCli)

-include $(DEPENDSLedLightSrv) 
-include $(DEPENDSLedLightCli)
include Utils/LedLight/ledApi/Makefile

Utils/LedLight/%.o: Utils/LedLight/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) $(INCPATHLedLight) -o $@ $<
	$(Q)$(CXX_SOFT) -MM -MF $(patsubst %.o,%.depend,$@) -MT $@ $(CXXFLAGS_SOFT) $(INCPATHLedLight) $<

Utils/LedLight/%.o: Utils/LedLight/%.c
	@echo Building $@
	$(Q)$(DISTCC_SOFT) -c $(CFLAGS_SOFT) $(INCPATHLedLight) -o $@ $<
	$(Q)$(CC_SOFT) -MM -MF $(patsubst %.o,%.depend,$@) -MT $@ $(CXXFLAGS_SOFT) $(INCPATHLedLight) $<

$(TARGETLedLightSrv): $(TARGETLedApi) $(OBJECTSLedLightSrv)
	@echo Building $@
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) $(OBJECTSLedLightSrv) \
                -L $(BIN_SOFT) -lninja_led -o $@

$(TARGETLedLightCli): $(TARGETLedApi) $(OBJECTSLedLightCli)
	@echo Building $@
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) $(OBJECTSLedLightCli) \
                -o $@
	
