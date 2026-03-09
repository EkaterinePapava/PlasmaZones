# COPR SRPM build instructions
# Called by COPR build system: make -f .copr/Makefile srpm outdir=<path>
# Also usable locally: make -f .copr/Makefile srpm outdir=./results
# SPDX-License-Identifier: GPL-3.0-or-later

SPEC := packaging/rpm/plasmazones.spec
NAME := plasmazones

# Extract version from the spec (CI stamps it; fallback to CMakeLists.txt)
VERSION := $(shell grep -m1 '^Version:' $(SPEC) | awk '{print $$2}')
ifeq ($(VERSION),0.0.0)
  VERSION := $(shell sed -n 's/^project(PlasmaZones VERSION \([^ ]*\).*/\1/p' CMakeLists.txt)
endif

srpm: $(SPEC)
	@echo "Building SRPM for $(NAME)-$(VERSION)"

	# Stamp version into spec
	sed -i 's/^Version:.*/Version:        $(VERSION)/' $(SPEC)

	# Generate changelog from CHANGELOG.md if the script exists
	@if [ -x packaging/generate-changelog.sh ]; then \
		bash packaging/generate-changelog.sh rpm; \
	fi

	# Create source tarball matching Source0 pattern in the spec
	mkdir -p /tmp/$(NAME)-srpm
	git archive --format=tar.gz --prefix=PlasmaZones-$(VERSION)/ HEAD \
		> /tmp/$(NAME)-srpm/$(NAME)-$(VERSION).tar.gz

	# Build SRPM
	rpmbuild -bs $(SPEC) \
		--define "_sourcedir /tmp/$(NAME)-srpm" \
		--define "_srcrpmdir $(outdir)" \
		--define "_topdir /tmp/$(NAME)-rpmbuild"

	@echo "SRPM written to $(outdir)"

.PHONY: srpm
