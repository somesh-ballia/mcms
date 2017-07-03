INCPATHDecompLog  = \
	-I Utils/DecompLog \
	-I Processes/Logger/LoggerLib/ \
	$(COMMONINCPATH) $(ZLIBINC_SOFT)

TARGETDecompLog_SOFT  = $(BIN_SOFT)/LogUtil
TARGETDecompLog_RMX   = $(BIN_RMX)/LogUtil

CSOURCESDecompLog := $(shell find Utils/DecompLog | grep -v /Tests/ | grep "\.c$$")
CPPSOURCESDecompLog := $(shell find Utils/DecompLog |  grep -v /Tests/ | grep "\.cpp$$")
SOURCESDecompLog := $(CSOURCESDecompLog) $(CPPSOURCESDecompLog)
OBJECTSDecompLog_RMX := $(CSOURCESDecompLog:.c=.o) $(CPPSOURCESDecompLog:.cpp=.$(RMX_PLATFORM).o)
OBJECTSDecompLog_SOFT := $(CSOURCESDecompLog:.c=.o) $(CPPSOURCESDecompLog:.cpp=.$(SOFT_PLATFORM).o)

ifndef SKIP_DEPEND
DEPENDSDecompLog := $(CSOURCESDecompLog:.c=.depend) $(CPPSOURCESDecompLog:.cpp=.depend)
endif

TARGETS_SOFT += $(TARGETDecompLog_SOFT)
TARGETS_RMX += $(TARGETDecompLog_RMX)

ifndef SKIP_DEPEND
DEPENDS += $(DEPENDSDecompLog)
-include Utils/*depend
endif

#-include $(DEPENDSDecompLog)

Utils/DecompLog/%.$(RMX_PLATFORM).o: Utils/DecompLog/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX_RMX) -c $(CXXFLAGS_RMX) $(INCPATHDecompLog) -o $@ $<

Utils/DecompLog/%.$(SOFT_PLATFORM).o: Utils/DecompLog/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) $(INCPATHDecompLog) -o $@ $<

ifndef SKIP_DEPEND
Utils/DecompLog/%.depend: Utils/DecompLog/%.cpp
	@echo Building $@
	$(Q)$(CC_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_SOFT) $(INCPATHDecompLog) $<
endif

$(TARGETDecompLog_SOFT):  $(OBJECTSDecompLog_SOFT)  $(DEPENDSDecompLog)
	@echo Building $@
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) $(OBJECTSDecompLog_SOFT) -L$(SYS_PACK_RMX)/zlib-1.2.7/  -lz -o $@

Utils/DecompLog/%.o: Utils/DecompLog/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX) -c $(CXXFLAGS) $(INCPATHDecompLog) -o $@ $<

#Utils/DecompLog/%.o: Utils/DecompLog/%.c
#	@echo Building $@
#	$(Q)$(DISTCC_RMX) -c $(CFLAGS_RMX) $(INCPATHHTTPUtils) -o $@ $<

$(TARGETDecompLog_RMX):  $(OBJECTSDecompLog_RMX) $(DEPENDSDecompLog)
	@echo Building $@
	$(Q)$(LINK_RMX) $(LFLAGS_RMX) $(OBJECTSDecompLog_RMX) -L$(SYS_PACK_RMX)/zlib-1.2.7/  -lz -o $@
