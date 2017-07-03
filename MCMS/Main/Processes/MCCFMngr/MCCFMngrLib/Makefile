# Makefile

TARGET   = MCCFMngrLib.a

SOURCES := $(shell ls -1 *.cpp)
OBJECTS := $(SOURCES:.cpp=.o)
DEPENDS := $(SOURCES:.cpp=.depend)

include ../../CommonProcessInclude.mk

INCPATH  = $(COMMONINCPATH) $(ENCRYPTINC)

include ../../../common.mk

all:  $(TARGET)

-include $(DEPENDS)

$(TARGET):  $(OBJECTS)
	@echo Building library $@
	@$(AR) $(TARGET) $(OBJECTS)
