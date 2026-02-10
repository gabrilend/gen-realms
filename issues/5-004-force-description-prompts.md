# 3-004: Narrative Generation Prompt

## Current Behavior
No story narration exists. Game is purely mechanical.

## Intended Behavior
The LLM generates narrative text describing game events:
- Turn-by-turn story progression
- Card plays described as fantasy actions
- Combat as epic battles
- Purchases as alliances or acquisitions
- Maintains consistent tone and world

## Suggested Implementation Steps

1. Create `src/llm/prompts/narrative.lua`
2. Design the system prompt for story voice:
   ```
   You are narrating an epic fantasy battle in Symbeline Realms.

   Style: High fantasy, dramatic but concise. Like a bard
   recounting a legendary battle.

   Rules:
   - Keep narration to 2-3 sentences per event
   - Reference card names as characters/forces
   - Build tension as authority drops
   - Celebrate dramatic plays
   ```
3. Design event-specific prompt templates:
   - Card play narration
   - Purchase narration
   - Attack narration
   - Turn summary
4. Implement `NarrativePrompt.generate(event_type, event_data)`
5. Implement `NarrativePrompt.parse_response(response)` - clean text
6. Add variety prompts to avoid repetition
7. Write tests for each event type

## Related Documents
- 3-002-game-state-serialization.md
- notes/vision

## Dependencies
- 3-001: LLM API Integration
- 3-002: Game State Serialization

## Example Narrations

**Card Play (Dire Bear):**
```
From the depths of the Thornwood, Lady Morgaine summons
a dire bear of ancient lineage. Its roar echoes across
the battlefield as it joins her growing pack.
```

**Attack:**
```
Lord Theron's constructs unleash a barrage of arcane bolts!
Lady Morgaine's authority wavers as 6 damage tears through
her defenses.
```

**Purchase:**
```
With gold enough to buy a kingdom, Lord Theron secures the
allegiance of the Master Artificer. His workshop shall
forge mighty weapons indeed.
```

## Acceptance Criteria
- [ ] Narration generated for all event types
- [ ] Tone is consistent fantasy style
- [ ] Card names appear naturally in story
- [ ] Text is concise (2-3 sentences)
- [ ] Variety maintained across turns
