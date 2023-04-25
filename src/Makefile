# Compiler flags
CCFLAGS := -Wall -Wextra -c -fmessage-length=0 -std=c11

# Find source files in PWD, assign object file names
SRCS := $(shell find -name "*.c")
OBJS := $(SRCS:%.c=%.o)

# Include object files if not targeting 'clean'
ifneq ($(MAKECMDGOALS), clean)
-include $(OBJS:%.o=%.d)
endif

# Compile object files and create dependency lists
%.o: %.c
	gcc $(CCFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o $@ $<

# Link into static library
libcutest.a: $(OBJS)
	ar rcu $@ $+
	ranlib $@

# 'all' build target
all: libcutest.a

# 'clean' build target
clean:
	-rm *.a *.d *.o