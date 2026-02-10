# 3-004a: Canvas Infrastructure

## Parent Issue
3-004: Browser Canvas Renderer

## Current Behavior
Canvas infrastructure is implemented with layout management and demo mode.

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
- [x] Canvas fills browser window
- [x] Resize events handled correctly
- [x] Layout regions calculated proportionally
- [x] Render loop runs at 60fps
- [x] No flicker or tearing

## Implementation Notes (2026-02-10)

### Files Created/Modified
- `assets/web/style.css` - Complete stylesheet with faction colors, loading states, tooltips
- `assets/web/canvas.js` - Layout management module (window.canvasLayout API)
- `assets/web/index.html` - Updated to use external CSS and canvas.js module

### Layout Regions
Implemented all required regions with proportional sizing:
- `status` - Top bar for player stats
- `tradeRow` - Top center for purchasable cards
- `hand` - Bottom center for player's hand
- `playerBases` - Bottom left for player's bases
- `opponentBases` - Top left for opponent's bases
- `narrative` - Right side panel for story text
- `playArea` - Center area for gameplay

### Canvas Layout API (window.canvasLayout)
```javascript
init(canvasId)          // Initialize canvas and layout
getContext()            // Get 2D rendering context
getLayout()             // Get layout regions object
getFactionColor(faction, type)  // Get faction color
getValueColor(valueType)        // Get value color
handleResize()          // Manually trigger resize
clear()                 // Clear with background color
drawRegionBorder(name, title, color)  // Draw region outline
getCardSize()           // Get card dimensions
updateFps(time)         // Update FPS counter
getFps()                // Get current FPS
startRenderLoop(callback)  // Start render loop
```

### Demo Mode
Added standalone demo mode that runs without Wasm:
- Displays layout regions with colored borders
- Renders placeholder cards in hand and trade row
- Shows FPS counter
- Useful for testing layout without building Wasm

### Design Decisions
1. Used `{ alpha: false }` on canvas context for better performance
2. Layout adapts card size for screens under 1000px width
3. Minimum canvas size enforced (800x600) to prevent layout issues
4. All colors centralized in FACTION_COLORS and VALUE_COLORS objects
5. Render loop uses requestAnimationFrame for smooth 60fps
