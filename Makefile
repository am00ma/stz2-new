.PHONY: all configure install test docs clean vars

SHELL    := /bin/bash
PRESET    = default
BUILD_DIR = build

all:
	@if [ ! -d "$(BUILD_DIR)" ]; then $(MAKE) configure; fi
	cmake --build --preset $(PRESET)

configure:
	cmake --preset $(PRESET)

install:
	cmake --install $(BUILD_DIR)

test:
	ctest --test-dir $(BUILD_DIR)/tests --preset $(PRESET) --rerun-failed --output-on-failure

docs:
	cd $(BUILD_DIR)/html; python -m http.server 5555

clean:
	rm -f -r $(BUILD_DIR)

vars:
	echo "$(PRESET): $(BUILD_DIR)"
