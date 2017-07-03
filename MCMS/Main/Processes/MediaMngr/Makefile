

#INCPATHMediaMngr  = -I Processes/MediaMngr/MediaMngrLib \
#					-I Processes/MediaMngr/MediaMngrLib/Channels/ \
#					$(COMMONINCPATH) $(MPLAPIINC) $(CSAPIINC) $(ZLIBINC) $(ENCRYPTINC)

#TARGETMediaMngr      = Bin/MediaMngr

#CSOURCESMediaMngr := $(shell find Processes/MediaMngr/MediaMngrLib | grep -v /Tests/ | grep "\.c$$")
#CPPSOURCESMediaMngr := $(shell find Processes/MediaMngr/MediaMngrLib |  grep -v /Tests/ | grep "\.cpp$$")
#SOURCESMediaMngr := $(CSOURCESMediaMngr) $(CPPSOURCESMediaMngr)
#OBJECTSMediaMngr := $(CSOURCESMediaMngr:.c=.o) $(CPPSOURCESMediaMngr:.cpp=.o)
#DEPENDSMediaMngr := $(CSOURCESMediaMngr:.c=.depend) $(CPPSOURCESMediaMngr:.cpp=.depend)

#TARGETS += $(TARGETMediaMngr)

#-include $(DEPENDSMediaMngr)

#Processes/MediaMngr/MediaMngrLib/%.o: Processes/MediaMngr/MediaMngrLib/%.cpp
#	@echo Building $@
#	$(Q)$(DISTCXX) -c $(CXXFLAGS) $(INCPATHMediaMngr) -o $@ $<
#	$(Q)$(CXX) -MM -MF $(patsubst %.o,%.depend,$@) -MT $@ $(CXXFLAGS) $(INCPATHMediaMngr) $<

#Processes/MediaMngr/MediaMngrLib/%.o: Processes/MediaMngr/MediaMngrLib/%.c
#	@echo Building $@
#	$(Q)$(DISTCC) -c $(CFLAGS) $(INCPATHMediaMngr) -o $@ $<
#	$(Q)$(CC) -MM -MF $(patsubst %.o,%.depend,$@) -MT $@ $(CXXFLAGS) $(INCPATHMediaMngr) $<

#Processes/MediaMngr/MediaMngrLib/Channels/%.o: Processes/MediaMngr/MediaMngrLib/Channels/%.cpp
#	@echo Building $@
#	$(Q)$(DISTCXX) -c $(CXXFLAGS) $(INCPATHMediaMngr) -o $@ $<
#	$(Q)$(CXX) -MM -MF $(patsubst %.o,%.depend,$@) -MT $@ $(CXXFLAGS) $(INCPATHMediaMngr) $<

#Processes/MediaMngr/MediaMngrLib/Channels/%.o: Processes/MediaMngr/MediaMngrLib/Channels/%.c
#	@echo Building $@
#	$(Q)$(DISTCC) -c $(CFLAGS) $(INCPATHMediaMngr) -o $@ $<
#	$(Q)$(CC) -MM -MF $(patsubst %.o,%.depend,$@) -MT $@ $(CXXFLAGS) $(INCPATHMediaMngr) $<

#$(TARGETMediaMngr):  $(OBJECTSMediaMngr) $(BASE_DEPEND) $(MAINOBJ) Bin/libMcmsEncryption.so
#	@echo Building $@
#	$(Q)$(LINK) $(LFLAGS) $(OBJECTSMediaMngr) $(BASE_LIBS) $(MAINOBJ) $(ENCRYPTLIB) -ldl -o $@


