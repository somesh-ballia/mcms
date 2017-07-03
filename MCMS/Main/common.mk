
.PHONY = all clean depend test

include /mcms/Main.mk


CC      = $(CROSS_COMPILE)gcc
CXX     = $(CROSS_COMPILE)g++
DISTCC  = distcc $(CROSS_COMPILE)gcc
DISTCXX = distcc $(CROSS_COMPILE)g++
FLAGS   = -pipe -pthread -g -m32 -march=i386 -DLINUX -rdynamic \
          -Wall -Wextra \
          -Wno-parentheses \
          -Wno-write-strings \
          -Wno-deprecated-declarations \
          -Wno-unused \
          -Wno-comment \
          -Wno-unknown-pragmas \
          -Wno-switch-enum \
          -Wno-switch \
          -Wno-ignored-qualifiers \
          $(MORE_FLAGS)

# Add -Werror on compilation without warnings
#
# -Wno-parentheses: warning: suggest parentheses around assignment used as truth value
# -Wno-deprecated-declarations: # warning: '<a function name>' defined but not used
# -Wno-unused: warning: unused parameter
# -Wno-comment: warning: multi-line comment
# -Wno-unknown-pragmas: warning: ignoring #pragma warning
# -Wno-switch-enum: enum value not handled in switch
# -Wno-reorder: warning: will be initialized after, only for C++
# -Wmissing-prototypes: warning: implicit declaration of function, only for C

# -march=i386 

# Avoids EOVERFLOW and O_LARGEFILE errors on onen a file whose size
#CC       =   /usr/bin/gcc
#CXX      =   /usr/bin/g++
DISTCC   = $(CCACHE_BIN) $(CROSS_COMPILE)gcc
DISTCXX  = $(CCACHE_BIN) $(CROSS_COMPILE)g++
LFLAGS   = -m32
# exceeds (2<<31)-1 bits
FLAGS += -D_FILE_OFFSET_BITS=64

ifeq ($(ENABLE_VERBOSE_OUTPUT),YES)
	Q=
else
	Q=@
endif


# Compiles with SSH on for debug
ifdef SSH_ON_FOR_DEBUG
  FLAGS += -DSSH_ON_FOR_DEBUG
endif
# compilation for Future RMX with CPU and Switch on Same Intel CPU
ifeq ("$(X86_COMPILATION)","YES")
	FLAGS += -D_X86_PLATFORM
else	
	FLAGS += -D_PPC_PLATFORM		
endif

# Compiles with optimization
ifdef COMPILE_OPTIMIZATION
  FLAGS += -O2 -Wno-uninitialized
else
  FLAGS += -O0
endif

CFLAGS         = $(FLAGS) -Wmissing-prototypes    #-std=c99 -pedantic
CXXFLAGS       = $(FLAGS) -Wno-reorder 						#-std=c++0x
LINK           = $(CROSS_COMPILE)g++
LIBS           = $(SUBLIBS) -lm 
CPPUNITLIB     = $(SYS_PACK)/cppunit-1.12.1/src/cppunit/.libs/libcppunit.a
AR             = $(CROSS_COMPILE)ar cqs
COPY           = cp -f
COPY_FILE      = $(COPY)
COPY_DIR       = $(COPY) -r
INSTALL_FILE   = $(COPY_FILE)
INSTALL_DIR    = $(COPY_DIR)
DEL_FILE       = rm -f
SYMLINK        = ln -sf
DEL_DIR        = rmdir
MOVE           = mv -f
CHK_DIR_EXISTS = test -d
MKDIR          = mkdir -p
MAKE           = make

first: all

# Implicit rules
.SUFFIXES: .c .o .cpp .depend

#.cpp.o:
#	@echo Building $@
#	$(DISTCXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<
#	$(CXX) -MM -MF $(patsubst %.o,%.depend,$@) $(CXXFLAGS) $(INCPATH) $<

#.c.o:
#	@echo Building $@
#	$(DISTCC) -c $(CFLAGS) $(INCPATH) -o $@ $<
#	$(CC) -MM -MF $(patsubst %.o,%.depend,$@) $(CFLAGS) $(INCPATH)  $<

#clean:
#	-$(DEL_FILE) $(OBJECTS) $(TARGET) $(TEST_OBJECTS) $(TEST_TARGET) tmp.$(TARGET)
#	-$(DEL_FILE) *.depend
#	-$(DEL_FILE) *~ core *.core
#	-$(DEL_FILE) *.keep *.keep.*
#	-$(DEL_FILE) *.contrib *.contrib.*
#	$(MORE_CLEAN)

#depend:
#	-$(DEL_FILE) `find  | grep  \.depend`
#	$(MORE_DEPEND)
