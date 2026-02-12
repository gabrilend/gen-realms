#!/bin/bash
# run-phase2-demo.sh - Launch Phase 2 network layer demonstration
#
# Builds and runs the Phase 2 demo which demonstrates:
# - Protocol message parsing and serialization
# - Session creation and management
# - Connection management (simulated)
# - Hidden information handling
# - Input validation
#
# Usage: ./scripts/run-phase2-demo.sh

# {{{ configuration
DIR="/mnt/mtwo/programming/ai-stuff/symbeline-realms"
if [ -n "$1" ]; then
    DIR="$1"
fi
# }}}

# {{{ main
cd "$DIR" || exit 1

echo "Building Phase 2 demo..."
if ! make demo2; then
    echo "Build failed!"
    exit 1
fi

echo ""
echo "Running Phase 2 demo..."
echo ""
./bin/phase-2-demo
# }}}
