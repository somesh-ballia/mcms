

TEST_TARGET = ../../Bin/$(PROCESS_NAME).Test

LIB_DIRECTORY = $(PROCESS_NAME)Lib
PROCESS_LIBS = $(LIB_DIRECTORY)/$(PROCESS_NAME)Lib.a
 
INCPATH  = -I . -I ../   -I$(LIB_DIRECTORY) \
	   -I /mcms/Libs/XmlPars \
	   -I /mcms/Libs/ProcessBase \
	   -I /mcms/Libs/Common \
	   -I /mcms/Libs/BFCPLib \
	   -I /mcms/Libs/SimLinux \
	   -I /mcms/IncludeInternalMcms \
	   -I /mcms/../McmIncld/Common \
	   -I /MCMS/Libs/NDMLib \
	   $(CPPUNITINC) \
	   $(MORE_INCLUDE_DIRS) \
	   -I$(LIB_DIRECTORY)


REQ_LIBS = $(MORE_LIBS)

DYN_LIBS = -lProcessBase -lXmlPars -lCommon -lSimLinux

SOURCES = ../Main.cpp
OBJECTS = $(SOURCES:.cpp=.o)
DEPENDS = $(SOURCES:.cpp=.depend)
TEST_SOURCES = $(shell ls -1 Tests/*.cpp 2>/dev/null) ../TestMain.cpp
TEST_OBJECTS = $(TEST_SOURCES:.cpp=.o)
TEST_DEPENDS =$(TEST_SOURCES:.cpp=.depend)

MORE_FLAGS = -D _CONSOLE -Wreturn-type

LINK_FLAGS = -lz -lrt -L../../Bin $(DYN_LIBS)

include ../../common.mk

MORE_CLEAN = cd $(LIB_DIRECTORY) >/dev/null ; $(MAKE) clean; cd - >/dev/null;

MORE_DEPEND = cd $(LIB_DIRECTORY) 2>/dev/null; $(MAKE) depend; cd - >/dev/null;


#all: makelib $(PROCESS_LIBS) $(TEST_TARGET) $(TARGET) $(STRIPPED_TARGET)
#just_process: $(PROCESS_LIBS) $(TEST_TARGET) $(TARGET) $(STRIPPED_TARGET)

all: makelib $(PROCESS_LIBS) $(TARGET)
just_process: $(PROCESS_LIBS) $(TARGET)



-include $(DEPENDS)
-include $(TEST_DEPENDS)

$(TARGET):  $(PROCESS_LIBS)  $(OBJECTS) $(REQ_LIBS)
	@echo Building $@
	$(LINKER) $(OBJECTS) $(PROCESS_LIBS) $(REQ_LIBS) $(MORE_DLL) \
	$(CXXFLAGS) $(MORE_FLAGS) $(LINK_FLAGS)  -o $@

$(TEST_TARGET):  $(PROCESS_LIBS) $(TEST_OBJECTS) $(REQ_LIBS)
	@echo Building $(TEST_TARGET)
	$(LINKER) $(TEST_OBJECTS) $(PROCESS_LIBS) $(REQ_LIBS) $(MORE_DLL) \
	$(CPPUNITLIB) $(CXXFLAGS) $(MORE_FLAGS) $(LINK_FLAGS) $(MORE_LINK_FLAGS_TEST) $(INCLUDE) -D _UNIT_TESTS -o $@

$(STRIPPED_TARGET): $(TARGET)
	@cp $(TARGET) $(STRIPPED_TARGET)
	strip -d $(STRIPPED_TARGET)


