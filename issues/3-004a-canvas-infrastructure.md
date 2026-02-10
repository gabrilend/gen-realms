# 3-004a: Canvas Infrastructure

## Parent Issue
3-004: Browser Canvas Renderer

## Current Behavior
No browser canvas setup exists.

## Intended Behavior
Core canvas infrastructure that:
- Creates and sizes HTML5 canvas element
- Handles window resize events
- Defines layout regions for game zones
- Provides drawing context management
- Implements render loop with requestAnimationFrame

## Suggested Implementation Steps

1. Create `assets/web/index.html`:
   ```html
   <!DOCTYPE html>
   <html>
   <head>
       <title>Symbeline Realms</title>
       <link rel="stylesheet" href="style.css">
   </head>
   <body>
       <canvas id="game-canvas"></canvas>
       <script src="canvas.js"></script>
       <script src="game.js"></script>
   </body>
   </html>
   ```

2. Create `assets/web/style.css`:
   ```css
   /* {{{ base styles */
   body {
       margin: 0;
       padding: 0;
       overflow: hidden;
       background: #1a1a2e;
   }

   #game-canvas {
       display: block;
       width: 100vw;
       height: 100vh;
   }
   /* }}} */
   ```

3. Create `assets/web/canvas.js` with initialization:
   ```javascript
   // {{{ canvas setup
   const canvas = document.getElementById('game-canvas');
   const ctx = canvas.getContext('2d');

   function resizeCanvas() {
       canvas.width = window.innerWidth;
       canvas.height = window.innerHeight;
       recalculateLayout();
   }

   window.addEventListener('resize', resizeCanvas);
   resizeCanvas();
   // }}}
   ```

4. Define layout regions:
   ```javascript
   // {{{ layout
   const layout = {
       hand: { x: 0, y: 0, w: 0, h: 0 },
       tradeRow: { x: 0, y: 0, w: 0, h: 0 },
       narrative: { x: 0, y: 0, w: 0, h: 0 },
       status: { x: 0, y: 0, w: 0, h: 0 },
       playerBases: { x: 0, y: 0, w: 0, h: 0 },
       opponentBases: { x: 0, y: 0, w: 0, h: 0 }
   };

   function recalculateLayout() {
       const w = canvas.width;
       const h = canvas.height;
       // Calculate proportional regions
       layout.status = { x: 0, y: 0, w: w, h: 50 };
       layout.tradeRow = { x: w * 0.5, y: 50, w: w * 0.5, h: h * 0.35 };
       // ... etc
   }
   // }}}
   ```

5. Implement render loop:
   ```javascript
   // {{{ render loop
   let lastTime = 0;
   function renderLoop(time) {
       const dt = time - lastTime;
       lastTime = time;

       ctx.clearRect(0, 0, canvas.width, canvas.height);
       renderGame(ctx, gameState, dt);
       requestAnimationFrame(renderLoop);
   }
   requestAnimationFrame(renderLoop);
   // }}}
   ```

6. Write basic layout tests

## Related Documents
- 3-004-browser-canvas-renderer.md (parent)
- 3-003-wasm-build-configuration.md

## Dependencies
- Modern browser with Canvas support

## Acceptance Criteria
- [ ] Canvas fills browser window
- [ ] Resize events handled correctly
- [ ] Layout regions calculated proportionally
- [ ] Render loop runs at 60fps
- [ ] No flicker or tearing
