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
- [ ] Static files serve correctly
- [ ] Wasm files have correct MIME type
- [ ] Index.html serves on root
- [ ] /api/config returns service endpoints
- [ ] 404 for missing files
