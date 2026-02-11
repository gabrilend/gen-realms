#!/bin/bash
# {{{ install-libssh.sh
# Downloads and compiles libssh from source.
#
# libssh is a C library implementing the SSH protocol for client and server.
# We build a static library with server support, using system OpenSSL for crypto.
#
# Usage:
#   ./install-libssh.sh           # Install/update if needed
#   ./install-libssh.sh --force   # Force reinstall
#   ./install-libssh.sh --prefix=/path  # Custom install location
#
# Environment:
#   SYMBELINE_DEPS_PREFIX - Override install location
#   BUILD_JOBS - Override parallel build count
#
# Requires:
#   System OpenSSL development headers (openssl-devel or libssl-dev)

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

# Check for OpenSSL
if ! has_command openssl; then
    warn "openssl command not found - build may fail"
fi

# Check for OpenSSL headers (try common locations)
OPENSSL_FOUND=false
for dir in /usr/include/openssl /usr/local/include/openssl; do
    if [ -f "${dir}/ssl.h" ]; then
        OPENSSL_FOUND=true
        break
    fi
done

if [ "${OPENSSL_FOUND}" = "false" ]; then
    warn "OpenSSL headers not found in standard locations"
    warn "Install openssl-devel (Void/Fedora) or libssl-dev (Debian/Ubuntu)"
fi
# }}}

# {{{ Version check
VERSION_FILE="${DEPS_INSTALL_PREFIX}/.libssh-version"
INSTALLED_VERSION=""

if [ -f "${VERSION_FILE}" ]; then
    INSTALLED_VERSION=$(cat "${VERSION_FILE}")
fi

if [ "${INSTALLED_VERSION}" = "${LIBSSH_VERSION}" ] && [ "${FORCE}" = "false" ]; then
    success "libssh ${LIBSSH_VERSION} already installed"
    exit 0
fi

if [ -n "${INSTALLED_VERSION}" ] && [ "${FORCE}" = "false" ]; then
    status "Upgrading libssh from ${INSTALLED_VERSION} to ${LIBSSH_VERSION}"
else
    status "Installing libssh ${LIBSSH_VERSION}"
fi
# }}}

# {{{ Create directories
mkdir -p "${DEPS_SOURCE_DIR}"
mkdir -p "${DEPS_INSTALL_PREFIX}"
# }}}

# {{{ Download source
SOURCE_DIR="${DEPS_SOURCE_DIR}/libssh"

if [ -d "${SOURCE_DIR}" ]; then
    status "Updating libssh source..."
    cd "${SOURCE_DIR}"
    git fetch --tags
else
    status "Cloning libssh..."
    git clone --depth 1 --branch "${LIBSSH_VERSION}" \
        "${LIBSSH_REPO}" "${SOURCE_DIR}"
    cd "${SOURCE_DIR}"
fi

# Checkout correct version
git checkout "${LIBSSH_VERSION}" 2>/dev/null || \
    git checkout "tags/${LIBSSH_VERSION}"
# }}}

# {{{ Build
BUILD_DIR="${SOURCE_DIR}/build"
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

status "Configuring libssh..."
cmake .. \
    -DCMAKE_INSTALL_PREFIX="${DEPS_INSTALL_PREFIX}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_STATIC_LIB=ON \
    -DBUILD_SHARED_LIBS=OFF \
    -DWITH_SERVER=ON \
    -DWITH_SFTP=OFF \
    -DWITH_EXAMPLES=OFF \
    -DWITH_GCRYPT=OFF \
    -DWITH_MBEDTLS=OFF \
    -DUNIT_TESTING=OFF \
    > cmake_output.log 2>&1 || {
        error "CMake configuration failed. See ${BUILD_DIR}/cmake_output.log"
        cat cmake_output.log >&2
        exit 1
    }

status "Building libssh (${BUILD_JOBS} jobs)..."
make -j${BUILD_JOBS} > build_output.log 2>&1 || {
    error "Build failed. See ${BUILD_DIR}/build_output.log"
    tail -50 build_output.log >&2
    exit 1
}
# }}}

# {{{ Install
status "Installing libssh..."
make install > install_output.log 2>&1 || {
    error "Install failed. See ${BUILD_DIR}/install_output.log"
    exit 1
}

# Write version file
echo "${LIBSSH_VERSION}" > "${VERSION_FILE}"
# }}}

# {{{ Verify installation
# libssh may install to lib or lib64
LIB_PATH=""
for libdir in lib lib64; do
    if [ -f "${DEPS_INSTALL_PREFIX}/${libdir}/libssh.a" ]; then
        LIB_PATH="${DEPS_INSTALL_PREFIX}/${libdir}/libssh.a"
        break
    fi
done

if [ -n "${LIB_PATH}" ]; then
    success "libssh ${LIBSSH_VERSION} installed to ${DEPS_INSTALL_PREFIX}"
    # Create symlink if in lib64
    if [[ "${LIB_PATH}" == *"lib64"* ]]; then
        mkdir -p "${DEPS_INSTALL_PREFIX}/lib"
        ln -sf "../lib64/libssh.a" "${DEPS_INSTALL_PREFIX}/lib/libssh.a"
        status "Created lib symlink for lib64"
    fi
else
    error "Installation verification failed - library not found"
    exit 1
fi
# }}}
# }}}
