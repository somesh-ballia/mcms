INCPATHNTP_bypass  = \
	-I Utils/NTP_bypass \
	$(COMMONINCPATH) 

TARGETNTP_bypass   =  $(BIN_RMX)/NTP_Bypass_Client

CSOURCESNTP_bypass := $(shell find Utils/NTP_bypass | grep -v /Tests/ | grep "\.c$$")
CPPSOURCESNTP_bypass := $(shell find Utils/NTP_bypass |  grep -v /Tests/ | grep "\.cpp$$")
SOURCESNTP_bypass := $(CSOURCESNTP_bypass) $(CPPSOURCESNTP_bypass)
OBJECTSNTP_bypass := $(CSOURCESNTP_bypass:.c=.o) $(CPPSOURCESNTP_bypass:.cpp=.o)

TARGETS_RMX += $(TARGETNTP_bypass)

ifndef SKIP_DEPEND
DEPENDSNTP_bypass := $(CSOURCESNTP_bypass:.c=.depend) $(CPPSOURCESNTP_bypass:.cpp=.depend)
DEPENDS += $(DEPENDSNTP_bypass)
-include Utils/NTP_bypass/*depend
endif

Utils/NTP_bypass/%.o: Utils/NTP_bypass/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX_RMX) -c $(CXXFLAGS_RMX) $(INCPATHNTP_bypass) -o $@ $<

Utils/NTP_bypass/%.o: Utils/NTP_bypass/%.c
	@echo Building $@
	$(Q)$(DISTCC_RMX) -c $(CFLAGS_RMX) $(INCPATHNTP_bypass) -o $@ $<

ifndef SKIP_DEPEND
Utils/NTP_bypass/%.depend: Utils/NTP_bypass/%.cpp
	@echo Building $@
	$(Q)$(CXX_RMX) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_RMX) $(INCPATHNTP_bypass) $<

Utils/NTP_bypass/%.depend: Utils/NTP_bypass/%.c
	@echo Building $@
	$(Q)$(CC) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CFLAGS_RMX) $(INCPATHNTP_bypass) $<
endif

$(TARGETNTP_bypass):  $(OBJECTSNTP_bypass) $(DEPENDSNTP_bypass)
	@echo Building $@
	$(Q)$(LINK_RMX) $(LFLAGS_RMX) $(OBJECTSNTP_bypass)  $(LINK_FLAGS_RMX) -lpthread -lrt -o $@
