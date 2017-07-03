INCPATHNTP_bypass_SoftMCU  = \
	-I Utils/NTP_bypass_SoftMCU \
	$(COMMONINCPATH) 

TARGETNTP_bypass_SoftMCU_SOFT   = $(BIN_SOFT)/NTP_Bypass_SoftMCU_Client $(BIN_SOFT)/NTP_Bypass_SoftMCU_Server $(BIN_SOFT)/NTP_Bypass_SoftMCU_Query

CSOURCESNTP_bypass_SoftMCU := $(shell find Utils/NTP_bypass_SoftMCU | grep -v /Tests/ | grep "\.c$$")
CPPSOURCESNTP_bypass_SoftMCU := $(shell find Utils/NTP_bypass_SoftMCU |  grep -v /Tests/ | grep "\.cpp$$")
SOURCESNTP_bypass_SoftMCU := $(CSOURCESNTP_bypass_SoftMCU) $(CPPSOURCESNTP_bypass_SoftMCU)
OBJECTSNTP_bypass_SoftMCU := $(CSOURCESNTP_bypass_SoftMCU:.c=.$(SOFT_PLATFORM).o) $(CPPSOURCESNTP_bypass_SoftMCU:.cpp=.$(SOFT_PLATFORM).o)

ifndef SKIP_DEPEND
DEPENDSNTP_bypass_SoftMCU := $(CSOURCESNTP_bypass_SoftMCU:.c=.depend) $(CPPSOURCESNTP_bypass_SoftMCU:.cpp=.depend)
endif

TARGETS_SOFT += $(TARGETNTP_bypass_SoftMCU_SOFT)

ifndef SKIP_DEPEND
DEPENDS +=  $(DEPENDSNTP_bypass_SoftMCU)
-include  Utils/NTP_bypass_SoftMCU/*depend
endif

Utils/NTP_bypass_SoftMCU/%.$(SOFT_PLATFORM).o: Utils/NTP_bypass_SoftMCU/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) $(INCPATHNTP_bypass_SoftMCU) -o $@ $<

Utils/NTP_bypass_SoftMCU/%.$(SOFT_PLATFORM).o: Utils/NTP_bypass_SoftMCU/%.c
	@echo Building $@
	$(Q)$(DISTCC_SOFT) -c $(CFLAGS_SOFT) $(INCPATHNTP_bypass_SoftMCU) -o $@ $<

ifndef SKIP_DEPEND
Utils/NTP_bypass_SoftMCU/%.depend: Utils/NTP_bypass_SoftMCU/%.cpp
	@echo Building $@
	$(Q)$(CXX_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_SOFT) $(INCPATHNTP_bypass_SoftMCU) $<

Utils/NTP_bypass_SoftMCU/%.depend: Utils/NTP_bypass_SoftMCU/%.c
	@echo Building $@
	$(Q)$(CC_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CFLAGS_SOFT) $(INCPATHNTP_bypass_SoftMCU) $<
endif

$(BIN_SOFT)/NTP_Bypass_SoftMCU_Client: Utils/NTP_bypass_SoftMCU/ntp_bypass_softmcu_client.$(SOFT_PLATFORM).o  $(DEPENDSNTP_bypass_SoftMCU)
	@echo Building $@ $(BIN_NEED_DEPEND)
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) Utils/NTP_bypass_SoftMCU/ntp_bypass_softmcu_client.$(SOFT_PLATFORM).o  $(LINK_FLAGS_SOFT) -lpthread -lrt -o $@

$(BIN_SOFT)/NTP_Bypass_SoftMCU_Server: Utils/NTP_bypass_SoftMCU/ntp_bypass_softmcu_server.$(SOFT_PLATFORM).o $(DEPENDSNTP_bypass_SoftMCU)
	@echo Building $@ $(BIN_NEED_DEPEND)
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT)  Utils/NTP_bypass_SoftMCU/ntp_bypass_softmcu_server.$(SOFT_PLATFORM).o  $(LINK_FLAGS_SOFT) -lpthread -lrt -o $@

$(BIN_SOFT)/NTP_Bypass_SoftMCU_Query: Utils/NTP_bypass_SoftMCU/ntp_bypass_softmcu_query.$(SOFT_PLATFORM).o $(DEPENDSNTP_bypass_SoftMCU)
	@echo Building $@ $(BIN_NEED_DEPEND)
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT)  Utils/NTP_bypass_SoftMCU/ntp_bypass_softmcu_query.$(SOFT_PLATFORM).o  $(LINK_FLAGS_SOFT) -lpthread -lrt -o $@
