#!/bin/bash
# run-phase3-demo.sh - Launch Phase 3 Client Renderer Demo
#
# This script builds and runs the Phase 3 demonstration which showcases
# the terminal (ncurses) client rendering capabilities.
#
# Run from any directory - script handles paths automatically.

# {{{ Configuration
DIR="${1:-/mnt/mtwo/programming/ai-stuff/symbeline-realms}"
# }}}

# {{{ setup
set -e
cd "$DIR"
# }}}

# {{{ build
echo "========================================"
echo "  Symbeline Realms - Phase 3 Demo"
echo "  Client Rendering Demonstration"
echo "========================================"
echo ""

echo "Building Phase 3 demo..."
make demo3

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi
# }}}

# {{{ run
echo ""
echo "Starting terminal client demo..."
echo "(Press 'q' to exit, 'h' for help)"
echo ""

./bin/phase-3-demo

exit_code=$?

echo ""
echo "Demo exited with code: $exit_code"

# }}}

exit $exit_code
