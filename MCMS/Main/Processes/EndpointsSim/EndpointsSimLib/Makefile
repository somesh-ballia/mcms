

TARGET   = EndpointsSimLib.a

SOURCES := $(shell ls -1 *.cpp)
OBJECTS := $(SOURCES:.cpp=.o)
DEPENDS := $(SOURCES:.cpp=.depend)

include ../../CommonProcessInclude.mk

INCPATH  = $(COMMONINCPATH) $(MPLAPIINC) $(CSAPIINC) $(ZLIBINC) \
	-I /mcms/Processes/ConfParty/ConfPartyLib/

include ../../../common.mk

all: $(TARGET)

-include $(DEPENDS)


$(TARGET):  $(OBJECTS)
	@echo Building library $@
	@$(AR) $(TARGET) $(OBJECTS)

