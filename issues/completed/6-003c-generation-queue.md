# 6-003c: Generation Queue

## Parent Issue
6-003: Dynamic Art Regeneration

## Current Behavior
No queue management for image generation.

## Intended Behavior
Queue system that:
- Batches generation requests
- Respects rate limits
- Prioritizes visible cards
- Handles failures with retry
- Shows generation progress

## Suggested Implementation Steps

1. Create queue structure:
   ```javascript
   // {{{ GenerationQueue class
   class GenerationQueue {
       constructor(options = {}) {
           this.queue = [];
           this.processing = false;
           this.maxConcurrent = options.maxConcurrent || 2;
           this.activeCount = 0;
           this.retryLimit = options.retryLimit || 3;
           this.retryDelay = options.retryDelay || 5000;
       }
   }
   // }}}
   ```

2. Implement priority levels:
   ```javascript
   // {{{ priority levels
   const PRIORITY = {
       VISIBLE_HAND: 1,      // Cards in player's hand
       VISIBLE_TRADE: 2,     // Cards in trade row
       VISIBLE_PLAY: 3,      // Cards currently in play
       BACKGROUND: 10        // Preload for next turn
   };

   function getPriority(card, gameState) {
       if (gameState.hand.includes(card)) return PRIORITY.VISIBLE_HAND;
       if (gameState.tradeRow.includes(card)) return PRIORITY.VISIBLE_TRADE;
       if (gameState.inPlay.includes(card)) return PRIORITY.VISIBLE_PLAY;
       return PRIORITY.BACKGROUND;
   }
   // }}}
   ```

3. Implement enqueue with priority:
   ```javascript
   // {{{ enqueue
   enqueue(card, prompt, priority) {
       const request = {
           card,
           prompt,
           priority,
           retries: 0,
           enqueuedAt: Date.now()
       };

       // Insert sorted by priority
       const index = this.queue.findIndex(r => r.priority > priority);
       if (index === -1) {
           this.queue.push(request);
       } else {
           this.queue.splice(index, 0, request);
       }

       this.processQueue();
   }
   // }}}
   ```

4. Implement queue processing:
   ```javascript
   // {{{ process queue
   async processQueue() {
       if (this.processing) return;
       this.processing = true;

       while (this.queue.length > 0 && this.activeCount < this.maxConcurrent) {
           const request = this.queue.shift();
           this.activeCount++;

           this.processRequest(request)
               .catch(err => this.handleFailure(request, err))
               .finally(() => {
                   this.activeCount--;
                   this.processQueue();
               });
       }

       this.processing = false;
   }
   // }}}
   ```

5. Implement request processing:
   ```javascript
   // {{{ process request
   async processRequest(request) {
       this.onProgress?.(request.card, 'generating');

       const image = await comfyuiClient.generate(request.prompt, {
           seed: request.card.image_seed
       });

       imageCache.set(request.card.instance_id, image);
       artTracker.markComplete(request.card);

       this.onProgress?.(request.card, 'complete');
   }
   // }}}
   ```

6. Implement retry logic:
   ```javascript
   // {{{ handle failure
   handleFailure(request, error) {
       console.error('Generation failed:', error);

       if (request.retries < this.retryLimit) {
           request.retries++;
           setTimeout(() => {
               this.queue.unshift(request);  // Add back to front
               this.processQueue();
           }, this.retryDelay * request.retries);
       } else {
           this.onError?.(request.card, error);
       }
   }
   // }}}
   ```

7. Add progress callbacks

8. Write tests for queue behavior

## Related Documents
- 6-003a-regeneration-tracking.md
- 6-001-comfyui-api-client.md

## Dependencies
- 6-001: ComfyUI API Client
- 6-003a: Regeneration Tracking

## Queue Behavior

```
Event: Player draws 5 cards, 3 need regeneration

Queue state:
1. [P1] Card A (hand) - generating
2. [P1] Card B (hand) - generating
3. [P1] Card C (hand) - waiting
   (max concurrent = 2)

Card A completes:
1. [P1] Card B (hand) - generating
2. [P1] Card C (hand) - generating

Trade row card D added:
1. [P1] Card B (hand) - generating
2. [P1] Card C (hand) - generating
3. [P2] Card D (trade) - waiting
```

## Acceptance Criteria
- [ ] Queue processes in priority order
- [ ] Concurrent limit respected
- [ ] Failures retry with backoff
- [ ] Progress callbacks fire
- [ ] Queue drains correctly
