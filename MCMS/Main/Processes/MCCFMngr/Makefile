
INCPATHMCCFMngr = -I Processes/MCCFMngr/MCCFMngrLib \
		$(COMMONINCPATH) 


TARGETMCCFMngr      = Bin/MCCFMngr

CSOURCESMCCFMngr := $(shell find Processes/MCCFMngr/MCCFMngrLib | grep -v /Tests/ | grep "\.c$$")
CPPSOURCESMCCFMngr := $(shell find Processes/MCCFMngr/MCCFMngrLib |  grep -v /Tests/ | grep "\.cpp$$")
SOURCESMCCFMngr := $(CSOURCESMCCFMngr) $(CPPSOURCESMCCFMngr)
OBJECTSMCCFMngr := $(CSOURCESMCCFMngr:.c=.o) $(CPPSOURCESMCCFMngr:.cpp=.o)
DEPENDSMCCFMngr := $(CSOURCESMCCFMngr:.c=.depend) $(CPPSOURCESMCCFMngr:.cpp=.depend)

TARGETS += $(TARGETMCCFMngr)

-include $(DEPENDSMCCFMngr)

Processes/MCCFMngr/MCCFMngrLib/%.o: Processes/MCCFMngr/MCCFMngrLib/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX) -c $(CXXFLAGS) $(INCPATHMCCFMngr) -o $@ $<
	$(Q)$(CXX) -MM -MF $(patsubst %.o,%.depend,$@) -MT $@ $(CXXFLAGS) $(INCPATHMCCFMngr) $<

Processes/MCCFMngr/MCCFMngrLib/%.o: Processes/MCCFMngr/MCCFMngrLib/%.c
	@echo Building $@
	$(Q)$(DISTCC) -c $(CFLAGS) $(INCPATHMCCFMngr) -o $@ $<
	$(Q)$(CC) -MM -MF $(patsubst %.o,%.depend,$@) -MT $@ $(CXXFLAGS) $(INCPATHMCCFMngr) $<

$(TARGETMCCFMngr):  $(OBJECTSMCCFMngr) $(BASE_DEPEND) $(MAINOBJ)
	@echo Building  $@
	$(Q)$(LINK) $(LFLAGS) $(OBJECTSMCCFMngr) $(BASE_LIBS) $(MAINOBJ) -o $@


TEST_TARGETMCCFMngr      = Bin/MCCFMngr.Test
TESTCPPSOURCESMCCFMngr := $(shell find Processes/MCCFMngr/Tests | grep "\.cpp$$")
TESTOBJECTSMCCFMngr := $(TESTCPPSOURCESMCCFMngr:.cpp=.o)
TESTDEPENDSMCCFMngr := $(TESTCPPSOURCESMCCFMngr:.cpp=.depend)

-include $(TESTDEPENDSMCCFMngr)

TEST_TARGETS += $(TEST_TARGETMCCFMngr)

Processes/MCCFMngr/Tests/%.o: Processes/MCCFMngr/Tests/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX) -c $(CXXFLAGS) $(INCPATHMCCFMngr)  $(CPPUNITINC)  -o $@ $<
	$(Q)$(CXX) -MM -MF $(patsubst %.o,%.depend,$@) -MT $@ $(CXXFLAGS) $(INCPATHMCCFMngr)  $(CPPUNITINC)  $<

$(TEST_TARGETMCCFMngr):  $(TARGETMCCFMngr) $(TESTMAINOBJ) $(TESTOBJECTSMCCFMngr) 
	@echo Building $@
	$(Q)$(LINK) $(LFLAGS) $(OBJECTSMCCFMngr) $(TESTMAINOBJ) $(TESTOBJECTSMCCFMngr) $(BASE_LIBS)  $(CPPUNITLIB) -o $@

