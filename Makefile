#
# GUID generator Makefile.
# @requires GNU make version >= v4.2
#
CC=$(TOOLCHAIN)gcc
CXX=$(TOOLCHAIN)g++
CXXFLAGS=-W -Wall -Wextra -pedantic -Os -std=c++17
LDFLAGS=-static -static-libstdc++ -static-libgcc
PROGRAM_NAME=guid-generate
BINARY=bin/$(PROGRAM_NAME)
GIT_COMMIT_HASH=$(shell git log -n1 --date=short --format='%h' .)
ARGS=

.PHONY: all binary clean run test

all: clean | binary

binary: $(BINARY)

clean:
	-@rm -f $(BINARY)
	-@rm -rf bin

run: $(BINARY)
	@echo "[run] Run without args".
	$(BINARY)
	@echo "[run] Run with args '$(ARGS)'"
	$(BINARY) $(ARGS)
	@echo "[run] done."

$(BINARY): src/main.cc
	@echo "[c++]" $@
	@mkdir -p $(dir $@)
	@$(CXX) -o $@ $< $(CXXFLAGS) $(LDFLAGS) -DPROGRAM_NAME='"$(PROGRAM_NAME)"' -DPROGRAM_VERSION='"#$(GIT_COMMIT_HASH)"'

#
# Minimal rng check, allows only to detect if something obviously is wrong.
# The application has to trust that the system random engine is good.
#
test:
	@echo "[test] Running random check ...".
	@test $(shell $(BINARY) -n1000000 | sort | uniq | wc -l) -eq 1000000 \
				&& echo "[pass] No rnd collision." \
				|| echo "[warn] Rnd collision, this is suspect for 128bit randomized values."
