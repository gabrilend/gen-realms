/*
 * 01-comfyui-client.h - ComfyUI API Client
 *
 * HTTP client for ComfyUI image generation API.
 * Submits workflow JSON, polls for completion, and retrieves generated images.
 * Used for dynamic card art and battle scene generation.
 */

#ifndef COMFYUI_CLIENT_H
#define COMFYUI_CLIENT_H

#include <stdbool.h>
#include <stddef.h>

// {{{ ComfyUIConfig
typedef struct {
    char* server_url;       // Server hostname (e.g., "localhost")
    int port;               // Server port (e.g., 8188)
    int timeout_ms;         // Request timeout in milliseconds
    int poll_interval_ms;   // Interval between status polls
    int max_poll_attempts;  // Maximum poll attempts before timeout
} ComfyUIConfig;
// }}}

// {{{ ComfyUIStatus
typedef enum {
    COMFYUI_STATUS_PENDING,     // Job queued, not started
    COMFYUI_STATUS_RUNNING,     // Job currently executing
    COMFYUI_STATUS_COMPLETED,   // Job finished successfully
    COMFYUI_STATUS_ERROR        // Job failed
} ComfyUIStatus;
// }}}

// {{{ ComfyUIResponse
typedef struct {
    char* prompt_id;        // Server-assigned job ID
    ComfyUIStatus status;   // Current job status
    unsigned char* image_data;  // Raw image bytes (PNG)
    size_t image_size;      // Size of image data
    char* error_message;    // Error details if failed
} ComfyUIResponse;
// }}}

// {{{ comfyui_config_create
// Creates a ComfyUIConfig with default values.
ComfyUIConfig* comfyui_config_create(void);
// }}}

// {{{ comfyui_config_free
// Frees all memory associated with config.
void comfyui_config_free(ComfyUIConfig* config);
// }}}

// {{{ comfyui_init
// Initializes the ComfyUI client. Call once at startup.
// Returns true on success, false on failure.
bool comfyui_init(void);
// }}}

// {{{ comfyui_cleanup
// Cleans up the ComfyUI client. Call once at shutdown.
void comfyui_cleanup(void);
// }}}

// {{{ comfyui_submit_workflow
// Submits a workflow JSON to ComfyUI for execution.
// Returns prompt_id on success, NULL on failure.
// Caller must free the returned string.
char* comfyui_submit_workflow(const ComfyUIConfig* config,
                               const char* workflow_json);
// }}}

// {{{ comfyui_get_status
// Gets the current status of a submitted job.
// Returns a response with status and any available results.
// Caller must free with comfyui_response_free.
ComfyUIResponse* comfyui_get_status(const ComfyUIConfig* config,
                                     const char* prompt_id);
// }}}

// {{{ comfyui_wait_for_completion
// Submits workflow and polls until completion or timeout.
// Returns response with image data on success.
// Caller must free with comfyui_response_free.
ComfyUIResponse* comfyui_wait_for_completion(const ComfyUIConfig* config,
                                              const char* workflow_json);
// }}}

// {{{ comfyui_get_image
// Retrieves a generated image by filename.
// Returns raw PNG bytes. Caller must free image_data.
unsigned char* comfyui_get_image(const ComfyUIConfig* config,
                                  const char* filename,
                                  size_t* size);
// }}}

// {{{ comfyui_response_free
// Frees all memory associated with response.
void comfyui_response_free(ComfyUIResponse* response);
// }}}

// {{{ comfyui_get_endpoint
// Returns formatted endpoint URL.
// Caller must free returned string.
char* comfyui_get_endpoint(const ComfyUIConfig* config);
// }}}

#endif /* COMFYUI_CLIENT_H */
