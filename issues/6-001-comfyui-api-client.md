# 6-001: ComfyUI API Client

## Current Behavior
No image generation capability exists.

## Intended Behavior
An HTTP client for ComfyUI API that:
- Connects to ComfyUI server
- Submits workflow JSON
- Polls for completion
- Retrieves generated images
- Handles errors and timeouts

## Suggested Implementation Steps

1. Create `src/visual/` directory structure
2. Create `src/visual/comfyui-client.h` with interface:
   ```c
   // {{{ comfyui types
   typedef struct {
       char* server_url;
       int port;
       int timeout_ms;
       int poll_interval_ms;
   } ComfyUIConfig;

   typedef struct {
       char* prompt_id;
       char* status;
       char* image_data;
       size_t image_size;
       char* error_message;
   } ComfyUIResponse;
   // }}}
   ```

3. Implement `comfyui_init()`:
   ```c
   // {{{ init
   ComfyUIConfig* comfyui_init(const char* url, int port) {
       ComfyUIConfig* cfg = malloc(sizeof(ComfyUIConfig));
       cfg->server_url = strdup(url);
       cfg->port = port;
       cfg->timeout_ms = 60000;
       cfg->poll_interval_ms = 500;
       return cfg;
   }
   // }}}
   ```

4. Implement `comfyui_submit_workflow()`:
   ```c
   // {{{ submit
   char* comfyui_submit_workflow(ComfyUIConfig* cfg, const char* workflow_json) {
       // POST to /prompt endpoint
       // Return prompt_id for polling
   }
   // }}}
   ```

5. Implement `comfyui_poll_status()`:
   ```c
   // {{{ poll
   ComfyUIResponse* comfyui_poll_status(ComfyUIConfig* cfg, const char* prompt_id) {
       // GET /history/{prompt_id}
       // Return status and image when complete
   }
   // }}}
   ```

6. Implement `comfyui_get_image()` to download result

7. Add WebSocket support for real-time progress

8. Write tests with mock server

## Related Documents
- 5-001-llm-api-client.md (similar HTTP pattern)
- docs/01-architecture-overview.md

## Dependencies
- libcurl (HTTP client)
- cJSON (JSON parsing)

## API Workflow

```
1. Client connects to ComfyUI (default: localhost:8188)
2. Submit workflow JSON to POST /prompt
3. Receive prompt_id
4. Poll GET /history/{prompt_id}
5. When complete, GET /view?filename={output}
6. Return image bytes to caller
```

## Acceptance Criteria
- [ ] Connects to ComfyUI server
- [ ] Submits workflow successfully
- [ ] Polls and detects completion
- [ ] Retrieves generated images
- [ ] Handles errors gracefully
