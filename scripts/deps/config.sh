#!/bin/bash
# {{{ config.sh
# Dependency configuration for Symbeline Realms.
#
# This file defines versions and paths for all external dependencies.
# Edit this file to customize dependency installation, or override
# settings via environment variables.
#
# Environment variable overrides:
#   SYMBELINE_DEPS_PREFIX - Override install location
#   BUILD_JOBS - Override parallel build count

# Resolve project root (two directories up from this script)
if [ -z "${PROJECT_ROOT}" ]; then
    PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
fi

# Where to install compiled dependencies
# Default: ${PROJECT_ROOT}/libs/local
# Override with SYMBELINE_DEPS_PREFIX environment variable
DEPS_INSTALL_PREFIX="${SYMBELINE_DEPS_PREFIX:-${PROJECT_ROOT}/libs/local}"

# Where to download/cache source code
# This directory is gitignored
DEPS_SOURCE_DIR="${PROJECT_ROOT}/libs/src"

# Dependency versions (stable releases)
# These are git tags from the respective repositories
LIBWEBSOCKETS_VERSION="v4.3.3"
LIBWEBSOCKETS_REPO="https://github.com/warmcat/libwebsockets.git"

LIBSSH_VERSION="libssh-0.10.6"
LIBSSH_REPO="https://git.libssh.org/projects/libssh.git"

# Build parallelism
# Default to number of CPU cores, or 4 if nproc unavailable
BUILD_JOBS="${BUILD_JOBS:-$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)}"

# Colors for output (disable with NO_COLOR=1)
if [ -z "${NO_COLOR}" ] && [ -t 1 ]; then
    COLOR_GREEN='\033[0;32m'
    COLOR_RED='\033[0;31m'
    COLOR_YELLOW='\033[0;33m'
    COLOR_BLUE='\033[0;34m'
    COLOR_RESET='\033[0m'
else
    COLOR_GREEN=''
    COLOR_RED=''
    COLOR_YELLOW=''
    COLOR_BLUE=''
    COLOR_RESET=''
fi

# Export for child scripts
export PROJECT_ROOT DEPS_INSTALL_PREFIX DEPS_SOURCE_DIR BUILD_JOBS
export LIBWEBSOCKETS_VERSION LIBWEBSOCKETS_REPO
export LIBSSH_VERSION LIBSSH_REPO
export COLOR_GREEN COLOR_RED COLOR_YELLOW COLOR_BLUE COLOR_RESET

# {{{ Helper functions

# Print status message
status() {
    echo -e "${COLOR_BLUE}>>>${COLOR_RESET} $1"
}

# Print success message
success() {
    echo -e "${COLOR_GREEN}[OK]${COLOR_RESET} $1"
}

# Print warning message
warn() {
    echo -e "${COLOR_YELLOW}[WARN]${COLOR_RESET} $1"
}

# Print error message
error() {
    echo -e "${COLOR_RED}[ERROR]${COLOR_RESET} $1" >&2
}

# Check if command exists
has_command() {
    command -v "$1" >/dev/null 2>&1
}

# Check required build tools
check_build_tools() {
    local missing=()

    if ! has_command cmake; then
        missing+=("cmake")
    fi

    if ! has_command make; then
        missing+=("make")
    fi

    if ! has_command gcc && ! has_command clang; then
        missing+=("gcc or clang")
    fi

    if ! has_command git; then
        missing+=("git")
    fi

    if [ ${#missing[@]} -gt 0 ]; then
        error "Missing required build tools:"
        for tool in "${missing[@]}"; do
            echo "  - ${tool}"
        done
        return 1
    fi

    return 0
}

export -f status success warn error has_command check_build_tools

# }}}
# }}}
