
ifndef SKIP_TESTS
INCPATHCommonLibTest  = -I Libs/Common \
					    -I Libs/Common/Tests \
					    -I Libs/XmlPars \
					    -I Libs/ProcessBase \
					    -I IncludeInternalMcms \
						-I Libs/SimLinux \
						-I Libs/BFCPLib \
						-I IncludeInternalMcms \
					    -I ../McmIncld/MPL/Card/PhisycalPortNetISDN \
					    -I ../McmIncld/Common \
			            -I ../McmIncld/CS/CsMaintenance \
			            -I ../McmIncld/CS/CsSignaling \
			            -I ../McmIncld/MPL/Card/CardMngrTB \
			            -I ../McmIncld/MPL/Card/CardMngrTIP \
					    $(CPPUNITINC)


TARGETCommonLibTest_SOFT = $(BIN_SOFT)/CommonLib.Test

SOURCESCommonLibTest := $(shell find Libs/Common/Tests | grep "\.cpp$$")
OBJECTSCommonLibTest := $(SOURCESCommonLibTest:.cpp=.o)
ifndef SKIP_DEPEND
DEPENDSCommonLibTest := $(SOURCESCommonLibTest:.cpp=.depend)
endif

TEST_TARGETS += $(TARGETCommonLibTest_SOFT)

ifndef SKIP_DEPEND
ifneq ($(MAKECMDGOALS) , clean)
-include $(DEPENDSCommonLibTest)
endif
endif


Libs/Common/Tests/%.o: Libs/Common/Tests/%.cpp
	@echo Building $@
	$(Q)$(DISTCC_SOFT) -c $(CFLAGS_SOFT) $(INCPATHCommonLibTest) -o $@ $<

ifndef SKIP_DEPEND

Libs/Common/Tests/%.depend: Libs/Common/Tests/%.cpp
	@echo Building $@
	$(Q)$(CXX_SOFT) -MM -MF $(patsubst %.o,%.depend,$@) -MT $@ $(CXXFLAGS) $(INCPATHCommonLibTest) $<

endif


$(TARGETCommonLibTest_SOFT): $(OBJECTSCommonLibTest) Bin/libProcessBase.so Bin/libXmlPars.so Bin/libCommon.so Bin/libSimLinux.so
	@echo Building $(TARGETCommonLibTest_SOFT)
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) $(OBJECTSCommonLibTest) $(CPPUNITLIB_SOFT) -lz -lrt -LBin $(DYN_LIBS) -lProcessBase -lXmlPars -lCommon -lSimLinux -o $@


endif

