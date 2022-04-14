CC ?= gcc
CFLAGS := -Wall -O3 -Wno-unused-function $(CFLAGS)
LDFLAGS := -lm

MAIN = raytracer
HEADERS = $(wildcard *.h entities/*.h lib/*.h)
SRCS = $(wildcard *.c entities/*.c lib/*.c)

OBJS_DIR = objs
OBJS = $(addprefix $(OBJS_DIR)/,$(filter-out $(MAIN).o,$(SRCS:.c=.o)))

$(MAIN): $(OBJS_DIR)/$(MAIN).o Makefile $(OBJS) $(HEADERS) .MAKE-LDFLAGS
	$(CC) $(CFLAGS) $(OBJS) $< -o $@ $(LDFLAGS)

$(OBJS_DIR)/%.o: %.c Makefile $(HEADERS) .MAKE-CFLAGS
	@mkdir -p $(OBJS_DIR)/lib $(OBJS_DIR)/entities
	$(CC) $(CFLAGS) $< -c -o $@

.PHONY: debug
debug:
	@CFLAGS="-O0 -g -fno-omit-frame-pointer" $(MAKE)

###############################################################################
# Misc rules
###############################################################################

.PHONY: FORCE

# The technique (and code) used here to trigger a new build on change of
# CFLAGS and/or LDFLAGS as appropriated comes from Git. See its Makefile, lines
# 2768 to 2782 at commit 88d915a634 ("A few fixes before -rc2", 2021-11-04)
# for the original source.

.MAKE-CFLAGS: FORCE
	@if ! test -e .MAKE-CFLAGS && test x"$(CFLAGS)" = x; then \
		touch .MAKE-CFLAGS; \
	elif test x"$(CFLAGS)" != x"`cat .MAKE-CFLAGS 2>/dev/null`"; then \
		echo >&2 "    * new build flags"; \
		echo "$(CFLAGS)" >.MAKE-CFLAGS; \
        fi

.MAKE-LDFLAGS: FORCE
	@if ! test -e .MAKE-LDFLAGS && test x"$(LDFLAGS)" = x; then \
		touch .MAKE-LDFLAGS; \
	elif test x"$(LDFLAGS)" != x"`cat .MAKE-LDFLAGS 2>/dev/null`"; then \
		echo >&2 "    * new link flags"; \
		echo "$(LDFLAGS)" >.MAKE-LDFLAGS; \
        fi

.PHONY: clean tags
clean:
	rm -rf $(MAIN) objs

tags: $(SRCS) $(HEADERS)
	rm -f $@
	ctags -o $@ $(SRCS) $(HEADERS)

