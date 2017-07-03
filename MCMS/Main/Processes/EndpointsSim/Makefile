


INCPATHEndpointsSim  = -I Processes/EndpointsSim/EndpointsSimLib \
					   $(COMMONINCPATH) $(MPLAPIINC) $(CSAPIINC) $(ZLIBINC) \
					   -I /mcms/Processes/ConfParty/ConfPartyLib/

TARGETEndpointsSim      = Bin/EndpointsSim

CSOURCESEndpointsSim := $(shell find Processes/EndpointsSim/EndpointsSimLib | grep -v /Tests/ | grep "\.c$$")
CPPSOURCESEndpointsSim := $(shell find Processes/EndpointsSim/EndpointsSimLib |  grep -v /Tests/ | grep "\.cpp$$")
SOURCESEndpointsSim := $(CSOURCESEndpointsSim) $(CPPSOURCESEndpointsSim)
OBJECTSEndpointsSim := $(CSOURCESEndpointsSim:.c=.o) $(CPPSOURCESEndpointsSim:.cpp=.o)
DEPENDSEndpointsSim := $(CSOURCESEndpointsSim:.c=.depend) $(CPPSOURCESEndpointsSim:.cpp=.depend)

TARGETS += $(TARGETEndpointsSim)

-include $(DEPENDSEndpointsSim)

Processes/EndpointsSim/EndpointsSimLib/%.o: Processes/EndpointsSim/EndpointsSimLib/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX) -c $(CXXFLAGS) $(INCPATHEndpointsSim) -o $@ $<
	$(Q)$(CXX) -MM -MF $(patsubst %.o,%.depend,$@) -MT $@ $(CXXFLAGS) $(INCPATHEndpointsSim) $<

Processes/EndpointsSim/EndpointsSimLib/%.o: Processes/EndpointsSim/EndpointsSimLib/%.c
	@echo Building $@
	$(Q)$(DISTCC) -c $(CFLAGS) $(INCPATHEndpointsSim) -o $@ $<
	$(Q)$(CC) -MM -MF $(patsubst %.o,%.depend,$@) -MT $@ $(CXXFLAGS) $(INCPATHEndpointsSim) $<

$(TARGETEndpointsSim):  $(OBJECTSEndpointsSim) $(BASE_DEPEND) Libs/CsXmlEngine/CsXmlLib.a $(MAINOBJ)
	@echo Building $@
	$(Q)$(LINK) $(LFLAGS) $(OBJECTSEndpointsSim) $(BASE_LIBS) Libs/CsXmlEngine/CsXmlLib.a $(MAINOBJ) -o $@

ifndef SKIP_TESTS
TEST_TARGETEndpointsSim      = Bin/EndpointsSim.Test
TESTCPPSOURCESEndpointsSim := $(shell find Processes/EndpointsSim/Tests | grep "\.cpp$$")
TESTOBJECTSEndpointsSim := $(TESTCPPSOURCESEndpointsSim:.cpp=.o)
TESTDEPENDSEndpointsSim := $(TESTCPPSOURCESEndpointsSim:.cpp=.depend)

-include $(TESTDEPENDSEndpointsSim)



TEST_TARGETS += $(TEST_TARGETEndpointsSim)


Processes/EndpointsSim/Tests/%.o: Processes/EndpointsSim/Tests/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX) -c $(CXXFLAGS) $(INCPATHEndpointsSim)  $(CPPUNITINC)  -o $@ $<
	$(Q)$(CXX) -MM -MF $(patsubst %.o,%.depend,$@) -MT $@ $(CXXFLAGS) $(INCPATHEndpointsSim)  $(CPPUNITINC)  $<

$(TEST_TARGETEndpointsSim):  $(TARGETEndpointsSim) $(TESTMAINOBJ) $(TESTOBJECTSEndpointsSim) 
	@echo Building $@
	$(Q)$(LINK) $(LFLAGS) $(OBJECTSEndpointsSim) $(TESTMAINOBJ) $(TESTOBJECTSEndpointsSim) $(BASE_LIBS) Libs/CsXmlEngine/CsXmlLib.a $(CPPUNITLIB) -o $@

endif





