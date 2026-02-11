/* 03-terminal-input.h - Terminal input handling for Symbeline Realms
 *
 * Provides command input, parsing, and history support for the terminal
 * client. Commands follow a simple format: action [target] [secondary].
 */

#ifndef TERMINAL_INPUT_H
#define TERMINAL_INPUT_H

#include "01-terminal.h"
#include <stdbool.h>

/* {{{ Command types
 * Actions the player can take during their turn.
 */
typedef enum {
    CMD_NONE = 0,        /* No command / empty input */
    CMD_PLAY,            /* Play a card from hand: p <index> */
    CMD_BUY,             /* Buy from trade row: b <index> */
    CMD_BUY_WANDERER,    /* Buy wanderer/explorer: b w */
    CMD_ATTACK_PLAYER,   /* Attack opponent directly: a */
    CMD_ATTACK_BASE,     /* Attack opponent's base: a <index> */
    CMD_SCRAP,           /* Scrap a card: s <index> */
    CMD_ACTIVATE,        /* Activate base ability: x <index> */
    CMD_DRAW_ORDER,      /* Set draw order: d 3,1,5,2,4 */
    CMD_END_TURN,        /* End turn: e */
    CMD_HELP,            /* Show help: h or ? */
    CMD_QUIT,            /* Quit game: q */
    CMD_UNKNOWN          /* Unrecognized command */
} CommandType;
/* }}} */

/* {{{ Command structure
 * Parsed command ready for game engine processing.
 */
#define MAX_DRAW_ORDER 10

typedef struct {
    CommandType type;
    int target;          /* Primary target index (-1 if none) */
    int secondary;       /* Secondary target index (-1 if none) */
    int draw_order[MAX_DRAW_ORDER];  /* For CMD_DRAW_ORDER */
    int draw_order_count;            /* Number of cards in draw order */
    char raw[256];       /* Original input for error messages */
} Command;
/* }}} */

/* {{{ Game phase for context-sensitive help */
typedef enum {
    PHASE_MAIN = 0,      /* Main phase - play cards, buy, attack */
    PHASE_DRAW_ORDER,    /* Draw order selection phase */
    PHASE_WAITING,       /* Waiting for opponent */
    PHASE_GAME_OVER      /* Game ended */
} GamePhase;
/* }}} */

/* {{{ Input history
 * Stores recent commands for up/down arrow navigation.
 */
#define INPUT_HISTORY_SIZE 20
#define INPUT_MAX_LEN 256

typedef struct {
    char entries[INPUT_HISTORY_SIZE][INPUT_MAX_LEN];
    int count;           /* Number of entries stored */
    int position;        /* Current position when navigating (-1 = new input) */
} InputHistory;
/* }}} */

/* {{{ Function declarations */

/* Input reading */
Command terminal_read_command(TerminalUI* ui, InputHistory* history);
void terminal_show_prompt(TerminalUI* ui, const char* prompt);

/* Command parsing */
Command command_parse(const char* input);
const char* command_type_to_string(CommandType type);
bool command_is_valid(Command* cmd);

/* Input history */
InputHistory* input_history_create(void);
void input_history_free(InputHistory* history);
void input_history_add(InputHistory* history, const char* input);
const char* input_history_prev(InputHistory* history);
const char* input_history_next(InputHistory* history);
void input_history_reset_position(InputHistory* history);

/* Help display */
void terminal_show_help(TerminalUI* ui);
void terminal_show_help_for_phase(TerminalUI* ui, GamePhase phase);
void terminal_show_error(TerminalUI* ui, const char* message);
void terminal_show_message(TerminalUI* ui, const char* message);

/* JSON protocol conversion */
char* command_to_json(Command* cmd);
void command_json_free(char* json);

/* Command validation with context */
bool command_valid_for_phase(Command* cmd, GamePhase phase);
const char* command_validation_error(Command* cmd, GamePhase phase);

/* Tab completion */
const char* command_complete(const char* partial, int* match_count);
void terminal_show_completions(TerminalUI* ui, const char* partial);

/* }}} */

#endif /* TERMINAL_INPUT_H */
