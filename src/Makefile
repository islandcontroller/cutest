# Custom definitions
CCDEFS := -DCUTEST_VERSION="\"${CUTEST_LIB_VERSION}\""

# Compiler flags
CCFLAGS := -Wall -Wextra -c -fmessage-length=0 -std=c11 $(CCDEFS)

# Find source files in PWD, assign object file names
LIBS := -lm
SRCS := $(shell find -name "*.c")
OBJS := $(SRCS:%.c=%.o)

# Include object files if not targeting 'clean'
ifneq ($(MAKECMDGOALS), clean)
-include $(OBJS:%.o=%.d)
endif

# Compile object files and create dependency lists
%.o: %.c
	gcc $(CCFLAGS) $(LIBS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o $@ $<

# Link into static library
libcutest.a: $(OBJS)
	ar rc $@ $+
	ranlib $@

# 'all' build target
all: libcutest.a

# 'clean' build target
clean:
	-rm *.a *.d *.o