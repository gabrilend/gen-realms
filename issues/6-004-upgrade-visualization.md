# 4-004: Inpainting API Integration

## Current Behavior
No image generation capability exists.

## Intended Behavior
Connect to inpainting API for image generation:
- Send canvas + mask + prompt
- Receive generated image region
- Support multiple backends (local, API)
- Handle errors and retries
- Manage generation queue

## Suggested Implementation Steps

1. Create `src/visual/inpaint-client.lua`
2. Define client interface:
   ```lua
   local InpaintClient = {
     endpoint = "",
     api_key = "",
     model = "",
     config = {
       strength = 0.8,
       guidance = 7.5,
       steps = 30
     }
   }
   ```
3. Implement `Inpaint.new(config)` - create client
4. Implement `Inpaint.generate(canvas, mask, prompt, refs)`:
   - Send request with image + mask
   - Include reference images if supported
   - Return generated image
5. Add request queuing for rate limits
6. Add retry logic for failures
7. Support local Stable Diffusion backend
8. Create config file for API settings
9. Write tests with mock responses

## Related Documents
- 3-001-llm-api-integration-module.md (similar pattern)

## Dependencies
- 4-001: Canvas State Manager
- 4-003: Reference Image Mapping

## API Request Example

```lua
local request = {
  image = canvas.image_data,      -- base image
  mask = region.mask,             -- white = generate
  prompt = "A dire bear charging through forest, fantasy art",
  negative_prompt = "sci-fi, spaceship, modern",
  reference_images = {refs.faction_style},
  strength = 0.8,
  guidance_scale = 7.5
}

local result = client:generate(request)
canvas:set_region(region.x, region.y, result.image)
```

## Acceptance Criteria
- [ ] Can connect to inpainting API
- [ ] Sends canvas + mask correctly
- [ ] Receives and processes result
- [ ] Handles API errors gracefully
- [ ] Queue prevents rate limiting
