# 0-001c: Meta-Install and Config System

## Parent Issue
0-001: Dependency Management System

## Current Behavior
No unified installation process for project dependencies.

## Intended Behavior
A meta-install script and configuration system that:
- Provides single-command project setup
- Allows customization via config file
- Calls individual dependency installers
- Reports overall status
- Handles errors gracefully

## Suggested Implementation Steps

1. Create `scripts/deps/config.sh`:
   ```bash
   #!/bin/bash
   # {{{ config.sh
   # Dependency configuration for Symbeline Realms.
   # Edit this file to customize dependency installation.

   # Resolve project root (two directories up from this script)
   if [ -z "${PROJECT_ROOT}" ]; then
       PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
   fi

   # Where to install compiled dependencies
   # Override with SYMBELINE_DEPS_PREFIX environment variable
   DEPS_INSTALL_PREFIX="${SYMBELINE_DEPS_PREFIX:-${PROJECT_ROOT}/libs/local}"

   # Where to download/cache source code
   DEPS_SOURCE_DIR="${PROJECT_ROOT}/libs/src"

   # Dependency versions (stable releases)
   LIBWEBSOCKETS_VERSION="v4.3.3"
   LIBSSH_VERSION="libssh-0.10.6"

   # Build parallelism
   BUILD_JOBS="${BUILD_JOBS:-$(nproc 2>/dev/null || echo 4)}"

   # Export for child scripts
   export PROJECT_ROOT DEPS_INSTALL_PREFIX DEPS_SOURCE_DIR BUILD_JOBS
   export LIBWEBSOCKETS_VERSION LIBSSH_VERSION
   # }}}
   ```

2. Create `scripts/install-deps.sh`:
   ```bash
   #!/bin/bash
   # {{{ install-deps.sh
   # Meta-installer for all Symbeline Realms dependencies.
   # Usage: ./install-deps.sh [--force] [--prefix=/path]
   #
   # This script calls each dependency installer and reports status.

   set -e

   DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
   source "${DIR}/deps/config.sh"

   echo "=================================="
   echo "Symbeline Realms Dependency Setup"
   echo "=================================="
   echo ""
   echo "Install prefix: ${DEPS_INSTALL_PREFIX}"
   echo "Source cache:   ${DEPS_SOURCE_DIR}"
   echo "Build jobs:     ${BUILD_JOBS}"
   echo ""

   # Pass through arguments
   ARGS="$@"

   # Track status
   FAILED=()

   # Install each dependency
   echo ">>> Installing libwebsockets..."
   if "${DIR}/deps/install-libwebsockets.sh" ${ARGS}; then
       echo "    libwebsockets: OK"
   else
       echo "    libwebsockets: FAILED"
       FAILED+=("libwebsockets")
   fi

   echo ""
   echo ">>> Installing libssh..."
   if "${DIR}/deps/install-libssh.sh" ${ARGS}; then
       echo "    libssh: OK"
   else
       echo "    libssh: FAILED"
       FAILED+=("libssh")
   fi

   # Report summary
   echo ""
   echo "=================================="
   if [ ${#FAILED[@]} -eq 0 ]; then
       echo "All dependencies installed successfully!"
       echo ""
       echo "Add to your build:"
       echo "  CFLAGS += -I${DEPS_INSTALL_PREFIX}/include"
       echo "  LDFLAGS += -L${DEPS_INSTALL_PREFIX}/lib"
   else
       echo "Some dependencies failed:"
       for dep in "${FAILED[@]}"; do
           echo "  - ${dep}"
       done
       exit 1
   fi
   # }}}
   ```

3. Create directories:
   - `scripts/deps/`
   - `libs/local/` (created by installers)
   - `libs/src/` (created by installers, gitignored)

4. Update `.gitignore`:
   ```
   libs/src/
   libs/local/
   ```

## Related Documents
- 0-001-dependency-management-system.md (parent)
- 0-001a-libwebsockets-install.md
- 0-001b-libssh-install.md
- 0-001d-makefile-local-deps.md

## Acceptance Criteria
- [ ] Config file defines all settings
- [ ] Meta-installer calls all dependency scripts
- [ ] Arguments pass through correctly
- [ ] Status reporting is clear
- [ ] Errors are handled gracefully
- [ ] SYMBELINE_DEPS_PREFIX environment variable works
