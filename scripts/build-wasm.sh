#!/bin/bash
# build-wasm.sh - Build Symbeline Realms WebAssembly client
#
# Compiles the browser client to WebAssembly using Emscripten.
# Run this script from the project root or specify DIR as an argument.
#
# Usage: ./scripts/build-wasm.sh [project-dir]

# {{{ configuration
DIR="${1:-/mnt/mtwo/programming/ai-stuff/symbeline-realms}"
EMSDK_PATH="${EMSDK:-$HOME/emsdk}"
# }}}

# {{{ helper functions
# {{{ error
error() {
    echo "ERROR: $1" >&2
    exit 1
}
# }}}

# {{{ info
info() {
    echo "==> $1"
}
# }}}
# }}}

# {{{ main
main() {
    # Verify project directory exists
    if [ ! -d "$DIR" ]; then
        error "Project directory not found: $DIR"
    fi

    cd "$DIR" || error "Cannot change to directory: $DIR"

    # Check for Emscripten
    if ! command -v emcc &> /dev/null; then
        info "Emscripten not in PATH, attempting to source environment..."

        if [ -f "$EMSDK_PATH/emsdk_env.sh" ]; then
            source "$EMSDK_PATH/emsdk_env.sh"
        else
            error "Emscripten SDK not found. Install it with:
    git clone https://github.com/emscripten-core/emsdk.git
    cd emsdk && ./emsdk install latest && ./emsdk activate latest

    Then set EMSDK environment variable or source emsdk_env.sh"
        fi
    fi

    # Verify emcc is now available
    if ! command -v emcc &> /dev/null; then
        error "emcc still not available after sourcing environment"
    fi

    info "Using Emscripten: $(emcc --version | head -n1)"

    # Create output directory
    mkdir -p "$DIR/assets/web"

    # Build based on argument
    case "${2:-release}" in
        debug)
            info "Building debug WebAssembly..."
            make -f Makefile.wasm debug
            ;;
        clean)
            info "Cleaning build artifacts..."
            make -f Makefile.wasm clean
            ;;
        *)
            info "Building release WebAssembly..."
            make -f Makefile.wasm
            ;;
    esac

    BUILD_STATUS=$?

    if [ $BUILD_STATUS -eq 0 ]; then
        info "Build successful!"
        info "Output files:"
        ls -lh "$DIR/assets/web/symbeline."* 2>/dev/null
    else
        error "Build failed with status $BUILD_STATUS"
    fi

    return $BUILD_STATUS
}
# }}}

main "$@"
