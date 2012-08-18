#=========================================================================
# Allos main makefile
#=========================================================================

LIB_NAME = allos

include Makefile.common

# List of all the module directories
MODULE_DIRS := $(shell ls -d */ | grep allo)

#--------------------------------------------------------------------------
# Rules
#--------------------------------------------------------------------------

help:
	@echo No rule specified.
	@echo The possible rules are:
	@echo "    all ......... build all modules found in this directory"
	@echo "    allocore .... build allocore"
	@echo "    alloutil .... build allocore utilities extension"
	@echo "    alloGLV ..... build allocore/GLV binding"
	@echo "    Gamma ....... build Gamma external"
	@echo "    GLV ......... build GLV external"
	@echo "    clean........ clean all modules found in this directory"

include Makefile.rules

all:
	@for v in $(MODULE_DIRS); do\
		$(MAKE) --no-print-directory -C $$v install DESTDIR=../$(BUILD_DIR) linkfile;\
	done

allocore: FORCE
	@$(MAKE) --no-print-directory -C $@ install DESTDIR=../$(BUILD_DIR) linkfile

alloutil: FORCE allocore
#	@$(MAKE) --no-print-directory -C $@ install BUILD_DIR=../$(BUILD_DIR) DESTDIR=../$(BUILD_DIR)
	@$(MAKE) --no-print-directory -C $@ install DESTDIR=../$(BUILD_DIR) linkfile

allonect: FORCE allocore alloutil
	@$(MAKE) --no-print-directory -C $@ install DESTDIR=../$(BUILD_DIR)


Gamma GLV: FORCE
	@$(MAKE) --no-print-directory -C ../$@ install DESTDIR=../$(LIB_NAME)/$(BUILD_DIR) linkfile

alloGLV: FORCE allocore GLV
	@$(MAKE) --no-print-directory -C $@ install DESTDIR=../$(BUILD_DIR)


clean:
	@for v in $(MODULE_DIRS); do\
		$(MAKE) --no-print-directory -C $$v clean;\
	done


# This attempts to determine what modules have been built by looking in the build dir
BUILT_MODULES := $(basename $(shell if [ -d $(BUILD_DIR)/lib/ ]; then ls $(BUILD_DIR)/lib/; fi))
BUILT_MODULES := $(subst lib,,$(BUILT_MODULES))
BUILT_MODULES := $(filter allo%, $(BUILT_MODULES))


-include $(addsuffix /Makefile.link, $(BUILT_MODULES))
LDFLAGS += -L$(BUILD_DIR)/lib/
include Makefile.buildandrun
