# Alternative GNU Make project makefile autogenerated by Premake

ifndef config
  config=release_shared64
endif

ifndef verbose
  SILENT = @
endif

.PHONY: clean prebuild

ifeq ($(config),release_static64)
TARGETDIR = scripts/src/modules
TARGET = $(TARGETDIR)/*.inc
  define BUILDCMDS
	@echo Running build commands
	./scripts/generateEmbedModules.sh
  endef
  define CLEANCMDS
	@echo Running clean commands
	rm -f src/modules/*.inc
	rm -f src/util/embed
  endef

else ifeq ($(config),release_shared64)
TARGETDIR = scripts/src/modules
TARGET = $(TARGETDIR)/*.inc
  define BUILDCMDS
	@echo Running build commands
	./scripts/generateEmbedModules.sh
  endef
  define CLEANCMDS
	@echo Running clean commands
	rm -f src/modules/*.inc
	rm -f src/util/embed
  endef

else ifeq ($(config),release_static32)
TARGETDIR = scripts/src/modules
TARGET = $(TARGETDIR)/*.inc
  define BUILDCMDS
	@echo Running build commands
	./scripts/generateEmbedModules.sh
  endef
  define CLEANCMDS
	@echo Running clean commands
	rm -f src/modules/*.inc
	rm -f src/util/embed
  endef

else ifeq ($(config),release_shared32)
TARGETDIR = scripts/src/modules
TARGET = $(TARGETDIR)/*.inc
  define BUILDCMDS
	@echo Running build commands
	./scripts/generateEmbedModules.sh
  endef
  define CLEANCMDS
	@echo Running clean commands
	rm -f src/modules/*.inc
	rm -f src/util/embed
  endef

else ifeq ($(config),debug_static64)
TARGETDIR = scripts/src/modules
TARGET = $(TARGETDIR)/*.inc
  define BUILDCMDS
	@echo Running build commands
	./scripts/generateEmbedModules.sh
  endef
  define CLEANCMDS
	@echo Running clean commands
	rm -f src/modules/*.inc
	rm -f src/util/embed
  endef

else ifeq ($(config),debug_shared64)
TARGETDIR = scripts/src/modules
TARGET = $(TARGETDIR)/*.inc
  define BUILDCMDS
	@echo Running build commands
	./scripts/generateEmbedModules.sh
  endef
  define CLEANCMDS
	@echo Running clean commands
	rm -f src/modules/*.inc
	rm -f src/util/embed
  endef

else ifeq ($(config),debug_static32)
TARGETDIR = scripts/src/modules
TARGET = $(TARGETDIR)/*.inc
  define BUILDCMDS
	@echo Running build commands
	./scripts/generateEmbedModules.sh
  endef
  define CLEANCMDS
	@echo Running clean commands
	rm -f src/modules/*.inc
	rm -f src/util/embed
  endef

else ifeq ($(config),debug_shared32)
TARGETDIR = scripts/src/modules
TARGET = $(TARGETDIR)/*.inc
  define BUILDCMDS
	@echo Running build commands
	./scripts/generateEmbedModules.sh
  endef
  define CLEANCMDS
	@echo Running clean commands
	rm -f src/modules/*.inc
	rm -f src/util/embed
  endef

else
  $(error "invalid configuration $(config)")
endif

DEPS=src/modules/*.wren
$(TARGET): $(DEPS)
	$(BUILDCMDS)

clean:
	$(CLEANCMDS)