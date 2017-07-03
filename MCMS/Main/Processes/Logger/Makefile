
INCPATHLogger  = -I Processes/Logger/LoggerLib \
					$(COMMONINCPATH) $(ZLIBINC) $(LOG4CXXINC)

TARGETLogger      = Bin/Logger

CSOURCESLogger := $(shell find Processes/Logger/LoggerLib | grep -v /Tests/ | grep "\.c$$")
CPPSOURCESLogger := $(shell find Processes/Logger/LoggerLib |  grep -v /Tests/ | grep "\.cpp$$")
SOURCESLogger := $(CSOURCESLogger) $(CPPSOURCESLogger)
OBJECTSLogger := $(CSOURCESLogger:.c=.o) $(CPPSOURCESLogger:.cpp=.o)
DEPENDSLogger := $(CSOURCESLogger:.c=.depend) $(CPPSOURCESLogger:.cpp=.depend)

TARGETS += $(TARGETLogger)

-include $(DEPENDSLogger)

Processes/Logger/LoggerLib/%.o: Processes/Logger/LoggerLib/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX) -c $(CXXFLAGS) $(INCPATHLogger) -o $@ $<
	$(Q)$(CXX) -MM -MF $(patsubst %.o,%.depend,$@) -MT $@ $(CXXFLAGS) $(INCPATHLogger) $<

Processes/Logger/LoggerLib/%.o: Processes/Logger/LoggerLib/%.c
	@echo Building $@
	$(Q)$(DISTCC) -c $(CFLAGS) $(INCPATHLogger) -o $@ $<
	$(Q)$(CC) -MM -MF $(patsubst %.o,%.depend,$@) -MT $@ $(CXXFLAGS) $(INCPATHLogger) $<

$(TARGETLogger):  $(OBJECTSLogger) $(BASE_DEPEND) $(MAINOBJ)
	@echo Building $@
	$(Q)$(LINK) $(LFLAGS) $(OBJECTSLogger) $(BASE_LIBS) $(LOG4CXXLIB) $(MAINOBJ) -o $@



TEST_TARGETLogger      = Bin/Logger.Test
TESTCPPSOURCESLogger := $(shell find Processes/Logger/Tests | grep "\.cpp$$")
TESTOBJECTSLogger := $(TESTCPPSOURCESLogger:.cpp=.o)
TESTDEPENDSLogger := $(TESTCPPSOURCESLogger:.cpp=.depend)

-include $(TESTDEPENDSLogger)

TEST_TARGETS += $(TEST_TARGETLogger)

Processes/Logger/Tests/%.o: Processes/Logger/Tests/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX) -c $(CXXFLAGS) $(INCPATHLogger)  $(CPPUNITINC) -o $@ $<
	$(Q)$(CXX) -MM -MF $(patsubst %.o,%.depend,$@) -MT $@ $(CXXFLAGS) $(INCPATHLogger)  $(CPPUNITINC) $<

$(TEST_TARGETLogger):  $(TARGETLogger) $(TESTMAINOBJ) $(TESTOBJECTSLogger) 
	@echo Building $@
	$(Q)$(LINK) $(LFLAGS) $(OBJECTSLogger) $(TESTMAINOBJ) $(TESTOBJECTSLogger) $(BASE_LIBS)  $(LOG4CXXLIB)  $(CPPUNITLIB) -o $@


