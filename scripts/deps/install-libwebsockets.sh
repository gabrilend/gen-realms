#!/bin/bash
# {{{ install-libwebsockets.sh
# Downloads and compiles libwebsockets from source.
#
# libwebsockets is a lightweight C library for WebSocket clients and servers.
# We build a minimal static library without SSL/zlib for portability.
#
# Usage:
#   ./install-libwebsockets.sh           # Install/update if needed
#   ./install-libwebsockets.sh --force   # Force reinstall
#   ./install-libwebsockets.sh --prefix=/path  # Custom install location
#
# Environment:
#   SYMBELINE_DEPS_PREFIX - Override install location
#   BUILD_JOBS - Override parallel build count

set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${DIR}/config.sh"

# {{{ Parse arguments
FORCE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --force)
            FORCE=true
            shift
            ;;
        --prefix=*)
            DEPS_INSTALL_PREFIX="${1#*=}"
            shift
            ;;
        --help|-h)
            echo "Usage: $0 [--force] [--prefix=/path]"
            echo ""
            echo "Options:"
            echo "  --force         Force reinstall even if up to date"
            echo "  --prefix=/path  Install to specified directory"
            exit 0
            ;;
        *)
            error "Unknown option: $1"
            exit 1
            ;;
    esac
done
# }}}

# {{{ Check prerequisites
if ! check_build_tools; then
    exit 1
fi
# }}}

# {{{ Version check
VERSION_FILE="${DEPS_INSTALL_PREFIX}/.libwebsockets-version"
INSTALLED_VERSION=""

if [ -f "${VERSION_FILE}" ]; then
    INSTALLED_VERSION=$(cat "${VERSION_FILE}")
fi

if [ "${INSTALLED_VERSION}" = "${LIBWEBSOCKETS_VERSION}" ] && [ "${FORCE}" = "false" ]; then
    success "libwebsockets ${LIBWEBSOCKETS_VERSION} already installed"
    exit 0
fi

if [ -n "${INSTALLED_VERSION}" ] && [ "${FORCE}" = "false" ]; then
    status "Upgrading libwebsockets from ${INSTALLED_VERSION} to ${LIBWEBSOCKETS_VERSION}"
else
    status "Installing libwebsockets ${LIBWEBSOCKETS_VERSION}"
fi
# }}}

# {{{ Create directories
mkdir -p "${DEPS_SOURCE_DIR}"
mkdir -p "${DEPS_INSTALL_PREFIX}"
# }}}

# {{{ Download source
SOURCE_DIR="${DEPS_SOURCE_DIR}/libwebsockets"

if [ -d "${SOURCE_DIR}" ]; then
    status "Updating libwebsockets source..."
    cd "${SOURCE_DIR}"
    git fetch --tags
else
    status "Cloning libwebsockets..."
    git clone --depth 1 --branch "${LIBWEBSOCKETS_VERSION}" \
        "${LIBWEBSOCKETS_REPO}" "${SOURCE_DIR}"
    cd "${SOURCE_DIR}"
fi

# Checkout correct version
git checkout "${LIBWEBSOCKETS_VERSION}" 2>/dev/null || \
    git checkout "tags/${LIBWEBSOCKETS_VERSION}"
# }}}

# {{{ Build
BUILD_DIR="${SOURCE_DIR}/build"
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

status "Configuring libwebsockets..."
cmake .. \
    -DCMAKE_INSTALL_PREFIX="${DEPS_INSTALL_PREFIX}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DLWS_WITH_STATIC=ON \
    -DLWS_WITH_SHARED=OFF \
    -DLWS_WITHOUT_TESTAPPS=ON \
    -DLWS_WITHOUT_TEST_SERVER=ON \
    -DLWS_WITHOUT_TEST_CLIENT=ON \
    -DLWS_WITHOUT_TEST_PING=ON \
    -DLWS_WITHOUT_BUILTIN_GETIFADDRS=ON \
    -DLWS_WITH_SSL=OFF \
    -DLWS_WITH_ZLIB=OFF \
    -DLWS_WITH_ZIP_FOPS=OFF \
    -DLWS_IPV6=OFF \
    -DLWS_UNIX_SOCK=ON \
    > cmake_output.log 2>&1 || {
        error "CMake configuration failed. See ${BUILD_DIR}/cmake_output.log"
        exit 1
    }

status "Building libwebsockets (${BUILD_JOBS} jobs)..."
make -j${BUILD_JOBS} > build_output.log 2>&1 || {
    error "Build failed. See ${BUILD_DIR}/build_output.log"
    exit 1
}
# }}}

# {{{ Install
status "Installing libwebsockets..."
make install > install_output.log 2>&1 || {
    error "Install failed. See ${BUILD_DIR}/install_output.log"
    exit 1
}

# Write version file
echo "${LIBWEBSOCKETS_VERSION}" > "${VERSION_FILE}"
# }}}

# {{{ Verify installation
if [ -f "${DEPS_INSTALL_PREFIX}/lib/libwebsockets.a" ]; then
    success "libwebsockets ${LIBWEBSOCKETS_VERSION} installed to ${DEPS_INSTALL_PREFIX}"
else
    error "Installation verification failed - library not found"
    exit 1
fi
# }}}
# }}}
