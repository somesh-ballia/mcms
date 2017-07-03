INCPATHHTTPUtils = \
	-I Utils/HTTPUtils \
	$(COMMONINCPATH)

TARGETHTTPUtils   =  $(BIN_RMX)/SyncTime

CSOURCESHTTPUtils := $(shell find Utils/HTTPUtils | grep -v /Tests/ | grep "\.c$$")
CPPSOURCESHTTPUtils := $(shell find Utils/HTTPUtils |  grep -v /Tests/ | grep "\.cpp$$")
SOURCESHTTPUtils := $(CSOURCESHTTPUtils) $(CPPSOURCESHTTPUtils)
OBJECTSHTTPUtils := $(CSOURCESHTTPUtils:.c=.o) $(CPPSOURCESHTTPUtils:.cpp=.o)

ifndef SKIP_DEPEND
DEPENDSHTTPUtils := $(CSOURCESHTTPUtils:.c=.depend) $(CPPSOURCESHTTPUtils:.cpp=.depend)
endif

TARGETS_RMX += $(TARGETHTTPUtils)
ifndef SKIP_DEPEND
DEPENDS += $(DEPENDSHTTPUtils)
-include Utils/*depend
endif

Utils/HTTPUtils/%.o: Utils/HTTPUtils/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX) -c $(CXXFLAGS) $(INCPATHHTTPUtils) -o $@ $<

Utils/HTTPUtils/%.o: Utils/HTTPUtils/%.c
	@echo Building $@
	$(Q)$(DISTCC_RMX) -c $(CFLAGS_RMX) $(INCPATHHTTPUtils) -o $@ $<

ifndef SKIP_DEPEND
Utils/HTTPUtils/%.depend: Utils/HTTPUtils/%.cpp
	@echo Building $@
	$(Q)$(CXX) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_RMX) $(INCPATHHTTPUtils) $<

Utils/HTTPUtils/%.depend: Utils/HTTPUtils/%.c
	@echo Building $@
	$(Q)$(CC_RMX) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_RMX) $(INCPATHHTTPUtils) $<
endif

$(TARGETHTTPUtils):  $(OBJECTSHTTPUtils) $(DEPENDSHTTPUtils)
	@echo Building $@
	$(Q)$(LINK_RMX) $(LFLAGS_RMX) $(OBJECTSHTTPUtils) -LBin.i32ptx  -o $@
