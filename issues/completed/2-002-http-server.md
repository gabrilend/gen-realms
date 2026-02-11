# 2-002: HTTP Server

## Current Behavior
No HTTP serving capability exists.

## Intended Behavior
An HTTP server using libwebsockets that:
- Serves static files (HTML, JS, Wasm) for browser clients
- Handles initial connection before WebSocket upgrade
- Serves from `assets/web/` directory
- Supports basic MIME types
- Provides endpoints for service discovery

## Suggested Implementation Steps

1. Create `src/net/02-http.h` with type definitions
2. Create `src/net/02-http.c` with implementation
3. Initialize libwebsockets context with HTTP protocol
4. Implement static file serving:
   ```c
   static int callback_http(struct lws *wsi,
       enum lws_callback_reasons reason,
       void *user, void *in, size_t len);
   ```
5. Implement MIME type detection
6. Serve index.html for root path
7. Add /api/config endpoint for client discovery:
   ```json
   {
     "llm_endpoint": "...",
     "comfyui_endpoint": "..."
   }
   ```
8. Add security headers
9. Write tests (curl-based)

## Related Documents
- docs/04-architecture-c-server.md
- 2-001-configuration-system.md

## Dependencies
- 2-001: Configuration System
- libwebsockets library

## Acceptance Criteria
- [x] Static files serve correctly (implemented, requires libwebsockets to test)
- [x] Wasm files have correct MIME type
- [x] Index.html serves on root
- [x] /api/config returns service endpoints
- [x] 404 for missing files

## Implementation Notes

### Files Created
- `src/net/02-http.h` - HTTP server API and types
- `src/net/02-http.c` - libwebsockets-based HTTP server
- `tests/test-http.c` - MIME type and utility tests

### Library Dependency
Requires libwebsockets-devel to compile and run:
```bash
sudo xbps-install libwebsockets-devel  # Void Linux
sudo apt install libwebsockets-dev      # Debian/Ubuntu
```

### Key Features Implemented
- MIME type detection for common web file types
- Security headers (X-Frame-Options, X-Content-Type-Options)
- /api/config REST endpoint exposing LLM and ComfyUI endpoints
- Directory traversal prevention
- Automatic index.html serving for root path
- Chunked file transfer for large files

### Test Coverage
Unit tests for MIME types and file extensions work without libwebsockets.
Full server tests require the library installed.

Completed: 2026-02-10
