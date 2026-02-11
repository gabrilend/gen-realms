#!/bin/bash
# run-phase1-demo.sh - Run the Phase 1 CLI demonstration
#
# This script builds and runs the Phase 1 demo which showcases all
# core game mechanics: cards, decks, combat, auto-draw, bases, spawning.

DIR="/mnt/mtwo/programming/ai-stuff/symbeline-realms"

# Allow override via argument
if [ -n "$1" ]; then
    DIR="$1"
fi

cd "$DIR" || exit 1

# Build if needed
if [ ! -f "bin/phase-1-demo" ]; then
    echo "Building Phase 1 demo..."
    make demo
    if [ $? -ne 0 ]; then
        echo "Build failed!"
        exit 1
    fi
fi

# Run the demo
./bin/phase-1-demo
