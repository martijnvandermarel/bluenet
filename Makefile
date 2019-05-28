#######################################################################################################################
# The build system uses CMake. All the automatically generated code falls under the Lesser General Public License
# (LGPL GNU v3), the Apache License, or the MIT license, your choice.
#
# Authors:         Crownstone B.V. team
# Creation date:   Feb. 26, 2017
# License:         LGPLv3, MIT, Apache (triple-licensed)
#
# Copyright © 2017 Crownstone B.V. (http://crownstone.rocks/team)
#######################################################################################################################

#!/bin/make -f

#######################################################################################################################
# Check for the existence of required environmental variables 
#######################################################################################################################

# The build directory should be defined as environmental variable. We do not assume a default build directory such
# as ./build for example.
ifndef BLUENET_BUILD_DIR
$(error BLUENET_BUILD_DIR not defined!)
endif

ifndef BLUENET_BIN_DIR
$(error BLUENET_BIN_DIR not defined!)
endif

ifndef BLUENET_CONFIG_DIR
$(error BLUENET_CONFIG_DIR not defined!)
endif

ifndef BLUENET_WORKSPACE_DIR
	# Do not throw error, but relative paths cannot be used.
endif

#######################################################################################################################
# Optional configuration parameters that will be set to defaults if not set before
#######################################################################################################################

# The verbosity parameter is used in the cmake build files to e.g. display the definitions used.

ifndef VERBOSITY
VERBOSITY=1
endif

# TODO: Why not just use the -j flag instead of introducing a new one? This should not be hardcoded.
# TODO: Use config file, then pass it on via CMAKE_BUILD_PARALLEL_LEVEL.

ifndef COMPILE_WITH_J_PROCESSORS
COMPILE_WITH_J_PROCESSORS=8
endif

#######################################################################################################################
# Additional configuration options
#######################################################################################################################

# We have a Makefile with some additional configuration options. Note that if the configurations options change, 
# cmake will force a rebuild of everything! That's why a COMPILATION_TIME macro, although useful, is not included 
# for the DEBUG_COMPILE_FLAGS. If we do a release we want to be sure we building the latest, hence then 
# COMPILATION_TIME as a macro is included.
#
# Also when the git hash changes this triggers a rebuild.

SOURCE_DIR=$(shell pwd)
COMPILATION_DAY=$(shell date --iso=date)
COMPILATION_TIME=$(shell date '+%H:%M')
GIT_BRANCH=$(shell git symbolic-ref --short -q HEAD)
GIT_HASH=$(shell git rev-parse --short=25 HEAD)

DEBUG_COMPILE_FLAGS=-DCOMPILATION_DAY="\"${COMPILATION_DAY}\"" \
			  -DVERBOSITY="${VERBOSITY}" \
			  -DGIT_BRANCH="\"${GIT_BRANCH}\"" \
			  -DGIT_HASH="\"${GIT_HASH}\"" \
			  -DCMAKE_BUILD_TYPE=Debug

RELEASE_COMPILE_FLAGS=-DCOMPILATION_DAY="\"${COMPILATION_DAY}\"" \
			  -DCOMPILATION_TIME="\"${COMPILATION_TIME}\"" \
			  -DVERBOSITY="${VERBOSITY}" \
			  -DGIT_BRANCH="\"${GIT_BRANCH}\"" \
			  -DCMAKE_BUILD_TYPE=MinSizeRel

# We copy the cmake files we need to the bluenet folder. This:
#   + reduces the risk that someone overwrites the Makefile by running cmake in-source on accident;
#   + makes it fairly easy to swap out CMakeLists.txt for unit tests on the host (would otherwise clobber the system).
#
# The timestamp of the CMakeLists.txt or other files is NOT used by cmake to define a re-build 

define cross-compile-target-prepare
	@cp conf/cmake/CMakeLists.txt .
	@cp conf/cmake/arm.toolchain.cmake .
	@cp conf/cmake/CMakeBuild.config.default .
	@cp conf/cmake/CMakeConfig.cmake .
	#@if [ ! -f "CMakeLists.txt" ]; then ln -s conf/cmake/CMakeLists.txt .; fi
	#@if [ ! -f "arm.toolchain.cmake" ]; then ln -s conf/cmake/arm.toolchain.cmake .; fi
	#@if [ ! -f "CMakeBuild.config.default" ]; then ln -s conf/cmake/CMakeBuild.config.default .; fi
	#@if [ ! -f "CMakeConfig.cmake" ]; then ln -s conf/cmake/CMakeConfig.cmake .; fi
endef

define cross-compile-target-cleanup
	printf "++ Clean up files that were copied to bluenet dir\n"
	@rm -f CMakeLists.txt
	@rm -f arm.toolchain.cmake
	@rm -f CMakeBuild.config.default
	@rm -f CMakeConfig.cmake
endef

define host-compile-target-prepare
	printf "++ Use CMakeList.host_target.txt as CMakeLists.txt\n"
	@cp conf/cmake/CMakeLists.host_target.txt CMakeLists.txt
	@cp conf/cmake/CMakeBuild.config.default .
	@cp conf/cmake/CMakeConfig.cmake .
endef

define host-compile-target-cleanup
	printf "++ Clean up files that were copied to bluenet dir\n"
	@rm -f CMakeLists.txt
	@rm -f CMakeBuild.config.default
	@rm -f CMakeConfig.cmake
endef

define bootloader-compile-target-prepare
	@cp bootloader/CMakeLists.txt .
	@cp conf/cmake/arm.toolchain.cmake .
	@cp conf/cmake/CMakeBuild.config.default .
	@cp conf/cmake/CMakeConfig.cmake .
endef

define bootloader-compile-target-cleanup
	printf "++ Clean up files that were copied to bluenet dir\n"
	@rm -f CMakeLists.txt
	@rm -f arm.toolchain.cmake
	@rm -f CMakeBuild.config.default
	@rm -f CMakeConfig.cmake
endef

#######################################################################################################################
# Targets
#######################################################################################################################

all:
	@echo "Please call make with cross-compile-target or host-compile target"
	@echo "It is recommended to use the scripts/bluenet.sh script"

.ONESHELL:
release:
	$(call cross-compile-target-prepare)
	@mkdir -p $(BLUENET_BUILD_DIR)
	@cd $(BLUENET_BUILD_DIR)
	@echo "++ Jumped to directory `pwd`"
	cmake $(RELEASE_COMPILE_FLAGS) \
		$(SOURCE_DIR) -DCMAKE_TOOLCHAIN_FILE=$(SOURCE_DIR)/arm.toolchain.cmake && make -j${COMPILE_WITH_J_PROCESSORS}
	result=$$?
	@echo "++ Result of make command (0 means success): $$result"
	$(call cross-compile-target-cleanup)
	return $$result

.ONESHELL:
cross-compile-target:
	$(call cross-compile-target-prepare)
	@mkdir -p $(BLUENET_BUILD_DIR)
	@cd $(BLUENET_BUILD_DIR) 
	@echo "++ Jumped to directory `pwd`"
	@rm -f log.txt
	@cmake $(DEBUG_COMPILE_FLAGS) \
		$(SOURCE_DIR) -DBOOTLOADER=0 -DCMAKE_TOOLCHAIN_FILE=$(SOURCE_DIR)/arm.toolchain.cmake && make -j${COMPILE_WITH_J_PROCESSORS}
	result=$$?
	@echo "++ Result of make command (0 means success): $$result"
	
#	ifeq ($(result),0)
	@echo "++ Copy binaries to ${BLUENET_BIN_DIR}\n"
	@mkdir -p "${BLUENET_BIN_DIR}"
	@cp $(BLUENET_BUILD_DIR)/crownstone.hex $(BLUENET_BUILD_DIR)/crownstone.bin $(BLUENET_BUILD_DIR)/crownstone.elf "$(BLUENET_BIN_DIR)"
#	endif

	$(call cross-compile-target-cleanup)
	return $$result

.ONESHELL:
bootloader-compile-target:
	$(call bootloader-compile-target-prepare)
	@mkdir -p $(BLUENET_BUILD_DIR)
	@cd $(BLUENET_BUILD_DIR)
	@echo "++ Jumped to directory `pwd`"
	@rm -f log.txt
	@cmake $(DEBUG_COMPILE_FLAGS) \
		$(SOURCE_DIR) -DBOOTLOADER=1 -DCMAKE_TOOLCHAIN_FILE=$(SOURCE_DIR)/arm.toolchain.cmake && make -j${COMPILE_WITH_J_PROCESSORS}
	result=$$?
	@echo "++ Result of make command (0 means success): $$result"
	
#	ifeq ($(result),0)
	@echo "++ Copy binaries to ${BLUENET_BIN_DIR}\n"
	@mkdir -p "${BLUENET_BIN_DIR}"
	@cp $(BLUENET_BUILD_DIR)/bootloader.hex $(BLUENET_BUILD_DIR)/bootloader.bin $(BLUENET_BUILD_DIR)/bootloader.elf "$(BLUENET_BIN_DIR)"
#	endif

	$(call bootloader-compile-target-cleanup)
	return $$result

host-compile-target:
	$(call host-compile-target-prepare)
	@mkdir -p $(BLUENET_BUILD_DIR)
	@cd $(BLUENET_BUILD_DIR) && cmake $(RELEASE_COMPILE_FLAGS) \
		$(SOURCE_DIR) && make -j${COMPILE_WITH_J_PROCESSORS}
	result=$$?
	@echo "++ Result of make command (0 means success): $$result"
	$(call host-compile-target-cleanup)
	return $$result

clean:
	$(call cross-compile-target-cleanup)
	$(call host-compile-target-cleanup)
	$(call bootloader-compile-target-cleanup)
	return $$result

clean-all:
	@mkdir -p $(BLUENET_BUILD_DIR)
	@cd $(BLUENET_BUILD_DIR) && make clean
	return $$result

list:
	@$(MAKE) -pRrq -f $(lastword $(MAKEFILE_LIST)) : 2>/dev/null | awk -v RS= -F: '/^# File/,/^# Finished Make data base/ {if ($$1 !~ "^[#.]") {print $$1}}' | sort | egrep -v -e '^[^[:alnum:]]' -e '^$@$$' | xargs

.PHONY: all cross-compile-target host-compile-target clean list release
