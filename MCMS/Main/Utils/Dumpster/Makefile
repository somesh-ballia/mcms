INCPATHDumpster  = -I Utils/Dumpster $(COMMONINCPATH)

TARGETDumpster_SOFT = $(BIN_SOFT)/Dumpster

CSOURCESDumpster := $(shell find Utils/Dumpster | grep -v /Tests/ | grep "\.c$$")
CPPSOURCESDumpster := $(shell find Utils/Dumpster |  grep -v /Tests/ | grep "\.cpp$$")
SOURCESDumpster := $(CSOURCESDumpster) $(CPPSOURCESDumpster)
OBJECTSDumpster := $(CSOURCESDumpster:.c=.o) $(CPPSOURCESDumpster:.cpp=.o)

ifndef SKIP_DEPEND
DEPENDSDumpster := $(CSOURCESDumpster:.c=.depend) $(CPPSOURCESDumpster:.cpp=.depend)
endif

TARGETS_SOFT += $(TARGETDumpster_SOFT)

ifndef SKIP_DEPEND
DEPENDS +=$(DEPENDSDumpster)
-include Utils/Dumpster/*depend
#-include $(DEPENDSDumpster)
endif

Utils/Dumpster/%.o: Utils/Dumpster/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) $(INCPATHDumpster) -o $@ $<

Utils/Dumpster/%.o: Utils/Dumpster/%.c
	@echo Building $@
	$(Q)$(DISTCC_SOFT) -c $(CFLAGS_SOFT) $(INCPATHDumpster) -o $@ $<

ifndef SKIP_DEPEND
Utils/Dumpster/%.depend: Utils/Dumpster/%.cpp
	@echo Building $@
	$(Q)$(CXX_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_SOFT) $(INCPATHDumpster) $<

Utils/Dumpster/%.depend: Utils/Dumpster/%.c
	@echo Building $@
	$(Q)$(CC_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_SOFT) $(INCPATHDumpster) $<
endif

$(TARGETDumpster_SOFT):  $(OBJECTSDumpster) $(DEPENDSDumpster)
	@echo Building $@
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) $(OBJECTSDumpster) -o $@
