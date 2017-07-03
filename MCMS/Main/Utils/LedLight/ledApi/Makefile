INCPATHLedApi  = -I Utils/LedLight/ledApi \
                                   $(COMMONINCPATH) $(ZLIBINC_SOFT)


TARGETLedApi   = $(BIN_SOFT)/libninja_led.so

CSOURCESLedApi := $(shell find Utils/LedLight/ledApi | grep tusb_led_api | grep "\.c$$")
CPPSOURCESLedApi := $(shell find Utils/LedLight/ledApi | grep tusb_led_api | grep "\.cpp$$")
SOURCESLedApi := $(CSOURCESLedApi) $(CPPSOURCESLedApi)
OBJECTSLedApi := $(CSOURCESLedApi:.c=.o) $(CPPSOURCESLedApi:.cpp=.o)
DEPENDSLedApi := $(CSOURCESLedApi:.c=.depend) $(CPPSOURCESLedApi:.cpp=.depend)

TARGETS_SOFT += $(TARGETLedApi)

-include $(DEPENDSLedApi)

#Utils/LedLight/ledApi/%.o: Utils/LedLight/ledApi/%.cpp
#	@echo Building $@
#	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) $(INCPATHLedApi) -o $@ $<
#	$(Q)$(CXX_SOFT) -MM -MF $(patsubst %.o,%.depend,$@) -MT $@ $(CXXFLAGS_SOFT) $(INCPATHLedApi) $<

#Utils/LedLight/ledApi/%.o: Utils/LedLight/ledApi/%.c
#	@echo Building $@
#	$(Q)$(DISTCC_SOFT) -c $(CFLAGS_SOFT) $(INCPATHLedApi) -o $@ $<
#	$(Q)$(CC_SOFT) -MM -MF $(patsubst %.o,%.depend,$@) -MT $@ $(CXXFLAGS_SOFT) $(INCPATHLedApi) $<

#$(TARGETLedApi):  $(OBJECTSLedApi)
#	@echo Building $@
#	 $(Q)$(LINK_SOFT) $(LFLAGS_SOFT) $(OBJECTSLedApi) \
#                -lusb -shared -o $@a

Utils/LedLight/ledApi/%.o: Utils/LedLight/ledApi/%.c
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) $(CXXFLAGS_SOFT) -o $@ -c $<

$(TARGETLedApi): $(OBJECTSLedApi)
	@echo Building $@
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) $(OBJECTSLedApi) -lusb -shared -o $@
