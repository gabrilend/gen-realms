# Symbeline Realms - Architecture Overview

## Core Concept

Symbeline Realms is a fantasy-themed deck-building card game inspired by Star Realms. The game transforms sci-fi ship cards into fantasy scenarios through an AI-powered narrative system.

## Major Components

### 1. Game Engine
The core game loop handling:
- Player turns and phases
- Card acquisition from trade row
- Combat resolution
- Authority (health) tracking
- Deck/discard pile management

### 2. LLM Dungeon Master
An AI system that:
- Manages trade row card placement with narrative intent
- Generates story text based on game events
- Maintains narrative consistency across turns
- Favors placing multiples of previously-drawn cards (singleton-encouragement)

### 3. Visual Generation System
Progressive image generation using:
- Programmatic inpainting on a battle canvas
- Reference images from card art
- Frame-by-frame battle scene development
- Fantasy reinterpretation of sci-fi imagery

### 4. Display System
Outputs story and game state to:
- Player display
- LLM context (for narrative continuity)

## Modified Game Mechanics

### Deck Flow Tracker (d10)
- Starts at 5
- Increments when buying a card
- Decrements when scrapping a card
- On rollover (10->0): gain +1 card draw permanently (tracked with d4)
- On underflow (0->9): lose -1 card draw permanently

### Auto-Draw Effects
- Cards with "draw a card" effects trigger automatically when drawn at turn start
- Effect visually "disappears" from the card until next shuffle
- Streamlines gameplay by eliminating play-draw-play sequences

### Base Spawning
- Bases deploy effects that materialize next turn
- Example: Castle spawns infantry cards into discard
- Spawned units have "draw a card" or "scrap for bonus" effects
- Scrapped units "regroup" at their origin base

### Card Generation
- Some cards produce other cards into discard pile
- Cards without draw/scrap effects cost 1 more (permanent power)

## Data Flow

```
[Card Database] --> [Game Engine] --> [LLM DM]
                         |                |
                         v                v
                   [Game State]    [Narrative Text]
                         |                |
                         v                v
                   [Visual Gen] --> [Display]
```

## Technology Stack

- Language: Lua (LuaJIT compatible)
- LLM Integration: API calls to language model
- Image Generation: Inpainting pipeline
- Display: TUI or graphical output
