INCPATHSimLinux  = \
	-I Libs/SimLinux \
	-I Libs/XmlPars \
	-I Libs/ProcessBase \
	-I Libs/Common \
	-I ../McmIncld/CS/CsSignaling \
	-I ../McmIncld/Common \
	-I IncludeInternalMcms \
	$(XML2INC)

INCPATHSimLinuxTest = $(INCPATHSimLinux)

TARGETSimLinux_RMX  = $(BIN_RMX)/libSimLinux.so
TARGETSimLinux_SOFT = $(BIN_SOFT)/libSimLinux.so
TARGETSimLinuxTest = $(BIN_SOFT)/SimLinux.Test

SOURCESSimLinux := $(shell find Libs/SimLinux |  grep -v /Tests/ | grep "\.cpp$$")
OBJECTSSimLinux_RMX := $(SOURCESSimLinux:.cpp=.$(RMX_PLATFORM).o)
OBJECTSSimLinux_SOFT := $(SOURCESSimLinux:.cpp=.$(SOFT_PLATFORM).o)
SOURCESSimLinuxTest := $(shell find Libs/SimLinux/Tests | grep "\.cpp$$")
OBJECTSSimLinuxTest := $(SOURCESSimLinuxTest:.cpp=.o)

ifndef SKIP_DEPEND
DEPENDSSimLinux := $(SOURCESSimLinux:.cpp=.depend)
DEPENDSSimLinuxTest := $(SOURCESSimLinuxTest:.cpp=.depend)
endif

TARGETS_RMX += $(TARGETSimLinux_RMX)
TARGETS_SOFT += $(TARGETSimLinux_SOFT)

ifndef SKIP_TESTS
TEST_TARGETS += $(TARGETSimLinuxTest)
endif

ifndef SKIP_DEPEND
DEPENDS += $(DEPENDSSimLinux) $(DEPENDSSimLinuxTest)
-include  Libs/SimLinux/depend
ifndef SKIP_TESTS
-include  Libs/SimLinux/Tests/depend
endif
endif

Libs/SimLinux/%.$(RMX_PLATFORM).o: Libs/SimLinux/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX_RMX) -c $(CXXFLAGS_RMX) $(INCPATHSimLinux) -o $@ $<

Libs/SimLinux/%.$(SOFT_PLATFORM).o: Libs/SimLinux/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) $(INCPATHSimLinux) -o $@ $<

ifndef SKIP_DEPEND
Libs/SimLinux/%.depend: Libs/SimLinux/%.cpp
	@echo Building $@
	$(Q)$(CXX_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_SOFT) $(INCPATHSimLinux) $<
endif

$(TARGETSimLinux_RMX):  $(OBJECTSSimLinux_RMX)  $(DEPENDSSimLinux)
	@echo Building library $@
	$(Q)$(LINK_RMX) $(LFLAGS_RMX) -o $@ $(OBJECTSSimLinux_RMX) $(STDLIB_RMX) $(XML2LIB) -lpthread  -shared

$(TARGETSimLinux_SOFT):  $(OBJECTSSimLinux_SOFT)  $(DEPENDSSimLinux) 
	@echo Building library $@
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) -o $@ $(OBJECTSSimLinux_SOFT) $(STDLIB_SOFT) $(XML2LIB) -lpthread  -shared

ifndef SKIP_TESTS
Libs/SimLinux/Tests/%.o: Libs/SimLinux/Tests/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) $(INCPATHSimLinuxTest) -o $@ $<
	$(Q)$(CXX_SOFT) -MM -MF $(patsubst %.o,%.depend,$@) -MT $(patsubst %.o,%.depend,$@) -MT $@ $(CXXFLAGS_SOFT) $(INCPATHSimLinuxTest) $<

$(TARGETSimLinuxTest): $(OBJECTSSimLinuxTest) $(BASE_DEPEND_SOFT)  $(DEPENDSSimLinux) $(DEPENDSSimLinuxTest)
	@echo Building $(TARGETSimLinuxTest)
	$(Q)$(LINK_SOFT) $(OBJECTSSimLinuxTest) $(CPPUNITLIB) $(INCPATHSimLinuxTest) $(LFLAGS_SOFT) $(BASE_LIBS_SOFT) $(CPPUNITLIB_SOFT) $(XML2LIB_SOFT) -o $@
endif
