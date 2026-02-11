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
    CMD_ATTACK_PLAYER,   /* Attack opponent directly: a */
    CMD_ATTACK_BASE,     /* Attack opponent's base: a <index> */
    CMD_SCRAP,           /* Scrap a card: s <index> */
    CMD_ACTIVATE,        /* Activate base ability: x <index> */
    CMD_END_TURN,        /* End turn: e */
    CMD_HELP,            /* Show help: h or ? */
    CMD_QUIT,            /* Quit game: q */
    CMD_UNKNOWN          /* Unrecognized command */
} CommandType;
/* }}} */

/* {{{ Command structure
 * Parsed command ready for game engine processing.
 */
typedef struct {
    CommandType type;
    int target;          /* Primary target index (-1 if none) */
    int secondary;       /* Secondary target index (-1 if none) */
    char raw[256];       /* Original input for error messages */
} Command;
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
void terminal_show_error(TerminalUI* ui, const char* message);
void terminal_show_message(TerminalUI* ui, const char* message);

/* }}} */

#endif /* TERMINAL_INPUT_H */
