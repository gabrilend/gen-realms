#!/bin/bash
# cleanup-legacy-js.sh - Remove legacy JavaScript files after WASM migration
#
# This script removes the old JavaScript files that have been replaced by
# pure WebAssembly C code. Run this only after verifying the new WASM client
# works correctly.
#
# Usage: ./scripts/cleanup-legacy-js.sh [--dry-run]

DIR="/mnt/mtwo/programming/ai-stuff/symbeline-realms"
WEB_DIR="$DIR/assets/web"

# Files to remove (replaced by WASM modules)
LEGACY_JS_FILES=(
    "canvas.js"
    "card-renderer.js"
    "zone-renderer.js"
    "panel-renderer.js"
    "input-handler.js"
    "preferences.js"
    "preferences-ui.js"
    "animation.js"
    "narrative.js"
    "card-animations.js"
    "effects.js"
    "draw-order.js"
    "art-tracker.js"
    "image-cache.js"
    "generation-queue.js"
    "style-merger.js"
    "upgrade-viz.js"
    "battle-canvas.js"
    "region-selector.js"
    "scene-composition.js"
    "style-transfer.js"
)

# Files to keep
# - symbeline.js: Emscripten-generated loader (will be regenerated)
# - index.html: Legacy client (keep for backward compatibility)
# - index-wasm.html: New pure WASM client
# - style.css: Legacy styles (keep for backward compatibility)

DRY_RUN=0
if [ "$1" == "--dry-run" ]; then
    DRY_RUN=1
    echo "=== DRY RUN MODE ==="
    echo ""
fi

echo "Legacy JavaScript Cleanup"
echo "========================="
echo ""
echo "The following files will be removed:"
echo ""

total_size=0
for file in "${LEGACY_JS_FILES[@]}"; do
    filepath="$WEB_DIR/$file"
    if [ -f "$filepath" ]; then
        size=$(wc -c < "$filepath")
        total_size=$((total_size + size))
        printf "  %-30s %6d bytes\n" "$file" "$size"
    else
        printf "  %-30s (not found)\n" "$file"
    fi
done

echo ""
echo "Total: $total_size bytes ($(echo "scale=1; $total_size/1024" | bc) KB)"
echo ""

if [ $DRY_RUN -eq 1 ]; then
    echo "Run without --dry-run to actually remove files."
    exit 0
fi

read -p "Proceed with removal? (y/N) " confirm
if [ "$confirm" != "y" ] && [ "$confirm" != "Y" ]; then
    echo "Aborted."
    exit 1
fi

echo ""
echo "Removing files..."
for file in "${LEGACY_JS_FILES[@]}"; do
    filepath="$WEB_DIR/$file"
    if [ -f "$filepath" ]; then
        rm "$filepath"
        echo "  Removed: $file"
    fi
done

echo ""
echo "Done. Legacy JavaScript files removed."
echo ""
echo "Don't forget to:"
echo "  1. Update index.html to use the new WASM client"
echo "  2. Remove style.css if no longer needed"
echo "  3. Commit the changes"
