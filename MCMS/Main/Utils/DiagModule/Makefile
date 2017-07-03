INCPATHDiagModule  = -I Utils/DiagModule \
                             -I Utils/DiagModule/includes \
                             -I Utils/DiagModule/sources/Diagnostics/includes \
                             -I Utils/DiagModule/sources/McmsCom/includes \
                             -I Utils/DiagModule/sources/StartUp/includes \
                             -I Utils/DiagModule/sources/EmaApi/includes \
                             -I Utils/DiagModule/sources/EmaCtl/includes \
                             -I Utils/DiagModule/sources/Ipmi/includes \
                             -I Utils/DiagModule/sources/Shared/includes \
                             -I Utils/DiagModule/sources/Tdd/includes \
                             $(ENCRYPTINC_SOFT) \
                             $(COMMONINCPATH)


TARGETDiagModule   = $(BIN_SOFT)/DiagModule

CSOURCESDiagModule := $(shell find Utils/DiagModule | grep "\.c$$")
SOURCESDiagModule := $(CSOURCESDiagModule)
OBJECTSDiagModule := $(CSOURCESDiagModule:.c=.o)
DEPENDSDiagModule := $(CSOURCESDiagModule:.c=.depend)
                                
TARGETDiagModule   = $(BIN_SOFT)/DiagModule

TARGETS_SOFT += $(TARGETDiagModule)

-include $(DEPENDSDiagModule)

Utils/DiagModule/%.o: Utils/DiagModule/%.c
	@echo Building $@
	$(Q)$(DISTCC_SOFT) -c $(CFLAGS_SOFT) -Dx86_ARCH -DDIAG_SSHD_START -DENABLE_CODEC_STRESS_TEST $(INCPATHDiagModule) -o $@ $<
	$(Q)$(CC_SOFT) -Dx86_ARCH -DDIAG_SSHD_START -DENABLE_CODEC_STRESS_TEST -MM -MF $(patsubst %.o,%.depend,$@) -MT $@ $(CFLAGS_SOFT) $(INCPATHDiagModule) $<

ENCRYPTLIB_SOFT_NO_MCMSENCRYPTION=`echo $(ENCRYPTLIB_SOFT) | sed 's/-lMcmsEncryption/ /'`

$(TARGETDiagModule):  $(OBJECTSDiagModule)
	@echo Building $@
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) $(OBJECTSDiagModule) $(ENCRYPTLIB_SOFT_NO_MCMSENCRYPTION) -lpthread -lrt -o $@


	
