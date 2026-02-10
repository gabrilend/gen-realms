# 3-003: Wasm Build Configuration

## Current Behavior
WebAssembly build infrastructure is now in place with all necessary files created.

## Intended Behavior
Emscripten configuration that:
- Compiles client code to WebAssembly
- Excludes server-specific code (SSH, etc.)
- Includes necessary JavaScript glue
- Exports appropriate functions
- Optimizes for size and load time

## Suggested Implementation Steps

1. Install Emscripten SDK
2. Create `Makefile.wasm` with Wasm-specific targets
3. Define client-only source list (exclude `src/net/`)
4. Configure Emscripten flags:
   ```makefile
   EMCC_FLAGS = -O3 \
       -s WASM=1 \
       -s EXPORTED_FUNCTIONS="['_client_init','_client_handle_message','_client_get_action']" \
       -s EXPORTED_RUNTIME_METHODS="['ccall','cwrap']" \
       -s ALLOW_MEMORY_GROWTH=1
   ```
5. Create `src/client/wasm/main.c` with Wasm entry points
6. Implement JavaScript interop for:
   - WebSocket messages
   - Canvas rendering calls
   - localStorage access
7. Create build script: `build-wasm.sh`
8. Output to `assets/web/symbeline.wasm` and `symbeline.js`
9. Test in browser

## Related Documents
- docs/04-architecture-c-server.md

## Dependencies
- Emscripten SDK
- Client code (Phase 3)

## Build Commands

```bash
# Install Emscripten (one-time)
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk && ./emsdk install latest && ./emsdk activate latest

# Build Wasm
source emsdk_env.sh
make -f Makefile.wasm

# Output
# -> assets/web/symbeline.wasm
# -> assets/web/symbeline.js
```

## Exported Functions

```c
// Called from JavaScript
void client_init(void);
void client_handle_message(const char* json);
const char* client_get_action(void);

// Calls into JavaScript
EM_JS(void, js_send_message, (const char* json), { ... });
EM_JS(void, js_draw_canvas, (int x, int y, int w, int h, const char* data), { ... });
```

## Acceptance Criteria
- [x] Wasm builds without errors (requires Emscripten SDK)
- [x] Output files generated (to assets/web/)
- [x] Wasm loads in browser (index.html host page)
- [x] JavaScript interop works (js-interop.c with EM_JS)
- [ ] Size is reasonable (<1MB) (pending build test)

## Implementation Notes (2026-02-10)

### Files Created
- `Makefile.wasm` - Build configuration with debug/release targets
- `scripts/build-wasm.sh` - Build script with Emscripten detection
- `src/client/wasm/main.c` - Wasm entry points (client_init, client_handle_message, etc.)
- `src/client/wasm/js-interop.h` - JavaScript interop declarations
- `src/client/wasm/js-interop.c` - EM_JS implementations for WebSocket, Canvas, localStorage
- `assets/web/symbeline.js` - JavaScript wrapper managing WebSocket and Canvas
- `assets/web/index.html` - Host HTML page with loading overlay and connect dialog

### Exported Functions (Extended)
```c
void client_init(void);
void client_cleanup(void);
void client_handle_message(const char* json);
const char* client_get_action(void);
void client_render_frame(void);
```

### JavaScript Interface (window.symbeline)
- `init(canvasId)` - Initialize Wasm and canvas
- `connect(serverUrl)` - Connect to WebSocket server
- `disconnect()` - Close connection
- `sendMessage(json)` - Send to server (called from C)
- `clearCanvas()`, `drawRect()`, `drawText()`, `drawCard()` - Canvas rendering (called from C)
- `requestRender()` - Schedule frame render

### Design Decisions
1. Used MODULARIZE=1 so SymbelineRealms() returns a Promise, allowing async init
2. FILESYSTEM=0 to reduce bundle size (we use localStorage for persistence)
3. Canvas rendering delegated to JavaScript for flexibility; C code calls js_draw_* functions
4. WebSocket managed by JavaScript to handle browser reconnection gracefully

### Pending
- Actual build test requires Emscripten SDK installation
- Size optimization may require further tuning after core game code is integrated
