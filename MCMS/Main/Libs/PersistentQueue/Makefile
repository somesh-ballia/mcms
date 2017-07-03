INCPATHPersistentQueue  = \
	-I Libs/PersistentQueue \
	-I Libs/ProcessBase \
	-I Libs/Common \
	-I Libs/CdrPersist \
	-I IncludeInternalMcms \
	-I ../McmIncld/Common \
	$(XML2INC)

INCPATHPersistentQueue_RMX = $(INCPATHPersistentQueue)
INCPATHPersistentQueue_SOFT = $(INCPATHPersistentQueue)

TARGETPersistentQueue_RMX = $(BIN_RMX)/libPersistentQueue.so
TARGETPersistentQueue_SOFT = $(BIN_SOFT)/libPersistentQueue.so

TARGETS_RMX += $(TARGETPersistentQueue_RMX)
TARGETS_SOFT += $(TARGETPersistentQueue_SOFT)

SOURCESPersistentQueue := $(shell find Libs/PersistentQueue |  grep -v /Tests/ | grep "\.cpp$$")
OBJECTSPersistentQueue_RMX := $(SOURCESPersistentQueue:.cpp=.$(RMX_PLATFORM).o)
OBJECTSPersistentQueue_SOFT := $(SOURCESPersistentQueue:.cpp=.$(SOFT_PLATFORM).o)

ifndef SKIP_DEPEND
DEPENDSPersistentQueue := $(SOURCESPersistentQueue:.cpp=.depend)
DEPENDS += $(DEPENDSPersistentQueue)
-include Libs/PersistentQueue/*depend
endif

PREREQUIRED_SOURCES += Libs/CdrPersist/generate.flag



Libs/PersistentQueue/%.$(RMX_PLATFORM).o: Libs/PersistentQueue/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX_RMX) -c $(CXXFLAGS_RMX) $(INCPATHPersistentQueue_RMX) -o $@ $<

Libs/PersistentQueue/%.$(SOFT_PLATFORM).o: Libs/PersistentQueue/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) $(INCPATHPersistentQueue_SOFT) -o $@ $<

ifndef SKIP_DEPEND
Libs/PersistentQueue/%.depend: Libs/PersistentQueue/%.cpp
	$(Q)$(CXX_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_SOFT) $(INCPATHPersistentQueue_SOFT) $< 
endif

$(TARGETPersistentQueue_RMX):  $(OBJECTSPersistentQueue_RMX) $(DEPENDSPersistentQueue)
	@echo Building library $@
	$(Q)$(LINK_RMX) $(LFLAGS_RMX) -o $@ $(OBJECTSPersistentQueue_RMX) -lpthread -shared 


$(TARGETPersistentQueue_SOFT): $(OBJECTSPersistentQueue_SOFT) $(DEPENDSPersistentQueue)
	@echo Building library $@
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) -o $@ $(OBJECTSPersistentQueue_SOFT) -lpthread -shared 

