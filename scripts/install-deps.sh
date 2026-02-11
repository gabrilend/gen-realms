#!/bin/bash
# {{{ install-deps.sh
# Meta-installer for all Symbeline Realms dependencies.
#
# This script installs all external dependencies from source, compiling
# them into the libs/local directory for a self-contained build.
#
# Usage:
#   ./install-deps.sh           # Install all dependencies
#   ./install-deps.sh --force   # Force reinstall all
#   ./install-deps.sh --prefix=/path  # Custom install location
#
# Environment:
#   SYMBELINE_DEPS_PREFIX - Override install location
#   BUILD_JOBS - Override parallel build count
#   NO_COLOR - Disable colored output

set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${DIR}/deps/config.sh"

# {{{ Print banner
echo ""
echo "==========================================="
echo "  Symbeline Realms Dependency Installer"
echo "==========================================="
echo ""
echo "  Install prefix: ${DEPS_INSTALL_PREFIX}"
echo "  Source cache:   ${DEPS_SOURCE_DIR}"
echo "  Build jobs:     ${BUILD_JOBS}"
echo ""
# }}}

# {{{ Parse arguments and pass through
ARGS="$@"

# Show help if requested
for arg in "$@"; do
    if [ "$arg" = "--help" ] || [ "$arg" = "-h" ]; then
        echo "Usage: $0 [--force] [--prefix=/path]"
        echo ""
        echo "Options:"
        echo "  --force         Force reinstall all dependencies"
        echo "  --prefix=/path  Install to specified directory"
        echo ""
        echo "Dependencies installed:"
        echo "  - libwebsockets ${LIBWEBSOCKETS_VERSION}"
        echo "  - libssh ${LIBSSH_VERSION}"
        echo ""
        echo "After installation, build with:"
        echo "  make deps-info  # Show detected dependencies"
        echo "  make            # Build project"
        exit 0
    fi
done
# }}}

# {{{ Check build tools first
status "Checking build tools..."
if ! check_build_tools; then
    error "Please install missing tools and try again"
    exit 1
fi
success "Build tools available"
echo ""
# }}}

# {{{ Track results
TOTAL=2
PASSED=0
FAILED=()
# }}}

# {{{ Install libwebsockets
echo "-------------------------------------------"
status "Installing libwebsockets..."
echo "-------------------------------------------"
if "${DIR}/deps/install-libwebsockets.sh" ${ARGS}; then
    ((PASSED++))
else
    FAILED+=("libwebsockets")
fi
echo ""
# }}}

# {{{ Install libssh
echo "-------------------------------------------"
status "Installing libssh..."
echo "-------------------------------------------"
if "${DIR}/deps/install-libssh.sh" ${ARGS}; then
    ((PASSED++))
else
    FAILED+=("libssh")
fi
echo ""
# }}}

# {{{ Summary
echo "==========================================="
echo "  Installation Summary"
echo "==========================================="
echo ""

if [ ${#FAILED[@]} -eq 0 ]; then
    success "All ${TOTAL} dependencies installed successfully!"
    echo ""
    echo "  Add to your build:"
    echo "    CFLAGS  += -I${DEPS_INSTALL_PREFIX}/include"
    echo "    LDFLAGS += -L${DEPS_INSTALL_PREFIX}/lib"
    echo ""
    echo "  Or simply run:"
    echo "    make deps-info  # Verify detection"
    echo "    make            # Build project"
    echo ""
    exit 0
else
    error "${#FAILED[@]} of ${TOTAL} dependencies failed:"
    for dep in "${FAILED[@]}"; do
        echo "    - ${dep}"
    done
    echo ""
    echo "  Check the logs in ${DEPS_SOURCE_DIR}/<dep>/build/"
    echo "  Try running with --force to clean rebuild"
    echo ""
    exit 1
fi
# }}}
# }}}
