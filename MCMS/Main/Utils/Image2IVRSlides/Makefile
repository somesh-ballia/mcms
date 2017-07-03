INCPATHImage2IVRSlides  = \
	-I Utils/Image2IVRSlides \
	-I Utils/Image2IVRSlides/libvslide/include \
	$(COMMONINCPATH)

TARGETGenerateIVRSlide_SOFT  = $(BIN_SOFT)/GenerateIVRSlide
TARGETGenerateIVRSlide_RMX   = $(BIN_RMX)/GenerateIVRSlide

TARGETImage2IVRSlides_SOFT  = $(BIN_SOFT)/ConvertImage2IVRSlides
TARGETImage2IVRSlides_RMX   = $(BIN_RMX)/ConvertImage2IVRSlides

CPPSOURCESGenerateIVRSlide := Utils/Image2IVRSlides/GenerateIVRSlide.cpp
OBJECTSGenerateIVRSlide_RMX := $(CPPSOURCESGenerateIVRSlide:.cpp=.$(RMX_PLATFORM).o)
OBJECTSGenerateIVRSlide_SOFT := $(CPPSOURCESGenerateIVRSlide:.cpp=.$(SOFT_PLATFORM).o)

CPPSOURCESImage2IVRSlides := Utils/Image2IVRSlides/ConvertImage2IVRSlides.cpp \
	Utils/Image2IVRSlides/Image2IVRSlidesConvertor.cpp

OBJECTSImage2IVRSlides_RMX := $(CPPSOURCESImage2IVRSlides:.cpp=.$(RMX_PLATFORM).o)
OBJECTSImage2IVRSlides_SOFT := $(CPPSOURCESImage2IVRSlides:.cpp=.$(SOFT_PLATFORM).o)

APP_DEP_LIBS:=Utils/Image2IVRSlides/libvslide/lib/libvideo_ivr_convert.a \
	Utils/Image2IVRSlides/libvslide/lib/svc_encoder_wrapper.a \
	Utils/Image2IVRSlides/libvslide/lib/rtv_encoder_wrapper.a \
	Utils/Image2IVRSlides/libvslide/lib/h26x_encoder_wrapper.a \
	Utils/Image2IVRSlides/libvslide/lib/libvideoencoder.a \
	Utils/Image2IVRSlides/libvslide/lib/libvideocommon.a \
	Utils/Image2IVRSlides/libvslide/lib/libvideoh26xcore.a \
	Utils/Image2IVRSlides/libvslide/lib/libvideoh26xenc.a \
	Utils/Image2IVRSlides/libvslide/lib/libcpudetect.a \
	Utils/Image2IVRSlides/libvslide/lib/libcolorconvertfrm.a \
	Utils/Image2IVRSlides/libvslide/lib/libinterlacedetect.a \
	Utils/Image2IVRSlides/libvslide/lib/libresize.a \
	Utils/Image2IVRSlides/libvslide/lib/libwmvcommon.a \
	Utils/Image2IVRSlides/libvslide/lib/libwmv9enc.a \
	Utils/Image2IVRSlides/libvslide/lib/libicp.a \
	Utils/Image2IVRSlides/libvslide/lib/libsvce32.a \
	Utils/Image2IVRSlides/libvslide/lib/libirc.a \
	Utils/Image2IVRSlides/libvslide/lib/libimf.a \
	Utils/Image2IVRSlides/libvslide/lib/libsvml.a \

ifdef USE_FREE_IMAGE
APP_DEP_LIBS:=$(APP_DEP_LIBS) Utils/Image2IVRSlides/libvslide/lib/libfreeimag.a
else
APP_DEP_LIBS:=$(APP_DEP_LIBS) Utils/Image2IVRSlides/libvslide/lib/libjpeg.a
endif

ifndef SKIP_DEPEND
DEPENDSGenerateIVRSlide := $(CPPSOURCESGenerateIVRSlide:.cpp=.depend)
DEPENDSImage2IVRSlides := $(CPPSOURCESImage2IVRSlides:.cpp=.depend)
endif

TARGETS_SOFT += $(TARGETGenerateIVRSlide_SOFT)
TARGETS_RMX += $(TARGETGenerateIVRSlide_RMX)

TARGETS_SOFT += $(TARGETImage2IVRSlides_SOFT)
TARGETS_RMX += $(TARGETImage2IVRSlides_RMX)

ifndef SKIP_DEPEND
DEPENDS += $(DEPENDSGenerateIVRSlide)
DEPENDS += $(DEPENDSImage2IVRSlides)
-include Utils/*depend
endif

#-include $(DEPENDSImage2IVRSlides)

Utils/Image2IVRSlides/%.$(RMX_PLATFORM).o: Utils/Image2IVRSlides/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX_RMX) -c $(CXXFLAGS_RMX) $(INCPATHImage2IVRSlides) -o $@ $<

Utils/Image2IVRSlides/%.$(SOFT_PLATFORM).o: Utils/Image2IVRSlides/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX_SOFT) -c $(CXXFLAGS_SOFT) $(INCPATHImage2IVRSlides) -o $@ $<

ifndef SKIP_DEPEND
Utils/Image2IVRSlides/%.depend: Utils/Image2IVRSlides/%.cpp
	@echo Building $@
	$(Q)$(CC_SOFT) -MM -MF $@ -MT $@ -MT $(patsubst %.depend,%.$(SOFT_PLATFORM).o,$@) -MT $(patsubst %.depend,%.$(RMX_PLATFORM).o,$@) $(CXXFLAGS_SOFT) $(INCPATHImage2IVRSlides) $<
endif

$(TARGETGenerateIVRSlide_SOFT):  $(OBJECTSGenerateIVRSlide_SOFT) $(DEPENDSGenerateIVRSlide)
	@echo Building $@
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) $(OBJECTSGenerateIVRSlide_SOFT) $(APP_DEP_LIBS) -lpthread -o $@

Utils/Image2IVRSlides/%.o: Utils/Image2IVRSlides/%.cpp
	@echo Building $@
	$(Q)$(DISTCXX) -c $(CXXFLAGS) $(INCPATHImage2IVRSlides) -o $@ $<

#Utils/Image2IVRSlides/%.o: Utils/Image2IVRSlides/%.c
#	@echo Building $@
#	$(Q)$(DISTCC_RMX) -c $(CFLAGS_RMX) $(INCPATHHTTPUtils) -o $@ $<

$(TARGETGenerateIVRSlide_RMX):  $(OBJECTSGenerateIVRSlide_RMX) $(DEPENDSGenerateIVRSlide)
	@echo Building $@
	$(Q)$(LINK_RMX) $(LFLAGS_RMX) $(OBJECTSGenerateIVRSlide_RMX) $(APP_DEP_LIBS) -lpthread -o $@


$(TARGETImage2IVRSlides_SOFT):  $(OBJECTSImage2IVRSlides_SOFT) $(DEPENDSImage2IVRSlides)
	@echo Building $@
	$(Q)$(LINK_SOFT) $(LFLAGS_SOFT) $(OBJECTSImage2IVRSlides_SOFT) -lpthread -o $@

$(TARGETImage2IVRSlides_RMX):  $(OBJECTSImage2IVRSlides_RMX) $(DEPENDSImage2IVRSlides)
	@echo Building $@
	$(Q)$(LINK_RMX) $(LFLAGS_RMX) $(OBJECTSImage2IVRSlides_RMX) -lpthread -o $@
    
    