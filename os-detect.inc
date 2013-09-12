# --- OS Selection
REAL_OS?=$(shell uname -s)
ifeq ($(REAL_OS), Darwin)
  REAL_OS = osx
else ifneq ($(findstring CYGWIN, $(REAL_OS)),)
  REAL_OS = win
endif

OS ?= $(REAL_OS)
ifeq ($(OS), Darwin)
  OS = osx
else ifneq ($(findstring WINDOWS, $(OS)),)
  OS = win
else ifeq ($(OS), Linux)
  OS = linux
endif
