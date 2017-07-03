INCPATHXml2Source = -I Utils/Xml2Source\
			$(XML2INC)\
			$(ZLIBINC_SOFT)\
			$(COMMONINCPATH) 

TARGETXml2Source_SOFT = $(XML2SOURCE_BIN)

CPPSOURCESXml2Source := $(shell find Utils/Xml2Source |  grep -v /Tests/ | grep "\.cpp$$")
SOURCESXml2Source := $(CPPSOURCESXml2Source)
OBJECTSXml2Source := $(CPPSOURCESXml2Source:.cpp=.o)

ifndef SKIP_DEPEND
DEPENDSXml2Source := $(CPPSOURCESXml2Source:.cpp=.depend)

ifneq ($(SKIP_CODE_GEN),YES)
	TARGETS_SOFT += $(TARGETXml2Source_SOFT)
	DEPENDS += $(DEPENDSXml2Source)
	-include Utils/Xml2Source/*depend
endif
endif

Utils/Xml2Source/%.o: Utils/Xml2Source/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) $(INCPATHXml2Source) -o $@ $<

ifndef SKIP_DEPEND
Utils/Xml2Source/%.depend: Utils/Xml2Source/%.cpp
	@echo Building $@
	$(Q)$(CXX_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.o,$@) $(CXXFLAGS_SOFT) $(INCPATHXml2Source) $<
endif

$(TARGETXml2Source_SOFT):  $(OBJECTSXml2Source) $(DEPENDSXml2Source)
	@echo Building $@
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) $(OBJECTSXml2Source) $(LINK_FLAGS_SOFT) -lxml2 -o $@
