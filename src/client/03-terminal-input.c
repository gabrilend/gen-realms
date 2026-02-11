/* 03-terminal-input.c - Terminal input handling implementation
 *
 * Implements command reading, parsing, and history navigation.
 * Uses ncurses for input handling with support for arrow key navigation.
 */

#include "03-terminal-input.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* {{{ Command strings for parsing */
static const struct {
    char key;
    const char* alt_key;
    CommandType type;
} COMMAND_KEYS[] = {
    { 'p', "play",   CMD_PLAY },
    { 'b', "buy",    CMD_BUY },
    { 'a', "attack", CMD_ATTACK_PLAYER },
    { 's', "scrap",  CMD_SCRAP },
    { 'x', "use",    CMD_ACTIVATE },
    { 'd', "draw",   CMD_DRAW_ORDER },
    { 'e', "end",    CMD_END_TURN },
    { 'h', "help",   CMD_HELP },
    { '?', NULL,     CMD_HELP },
    { 'q', "quit",   CMD_QUIT },
    { 0, NULL, CMD_NONE }
};

/* Command names for completion */
static const char* COMMAND_NAMES[] = {
    "play", "buy", "attack", "scrap", "use", "draw", "end", "help", "quit", NULL
};
/* }}} */

/* {{{ parse_draw_order
 * Helper to parse comma-separated draw order: "3,1,5,2,4"
 */
static int parse_draw_order(const char* str, int* order, int max_count) {
    int count = 0;
    const char* p = str;

    while (*p && count < max_count) {
        /* Skip whitespace and commas */
        while (*p && (isspace(*p) || *p == ',')) p++;
        if (*p == '\0') break;

        /* Parse number */
        char* end;
        long val = strtol(p, &end, 10);
        if (end == p) break;  /* No number found */

        order[count++] = (int)val;
        p = end;
    }

    return count;
}
/* }}} */

/* {{{ command_parse
 * Parse a command string into a Command structure.
 * Format: <action> [target] [secondary]
 * Examples: "p 2", "b 0", "b w", "a 1", "d 3,1,5,2", "e", "h"
 */
Command command_parse(const char* input) {
    Command cmd = { CMD_NONE, -1, -1, {0}, 0, "" };

    if (input == NULL || input[0] == '\0') {
        return cmd;
    }

    /* Store raw input for error messages */
    strncpy(cmd.raw, input, sizeof(cmd.raw) - 1);
    cmd.raw[sizeof(cmd.raw) - 1] = '\0';

    /* Skip leading whitespace */
    while (*input && isspace(*input)) input++;

    if (*input == '\0') {
        return cmd;
    }

    /* Check for single-character commands */
    char first = tolower(input[0]);

    for (int i = 0; COMMAND_KEYS[i].key != 0; i++) {
        if (first == COMMAND_KEYS[i].key) {
            cmd.type = COMMAND_KEYS[i].type;
            break;
        }
    }

    /* If not found, check for word commands */
    if (cmd.type == CMD_NONE) {
        char word[32];
        if (sscanf(input, "%31s", word) == 1) {
            for (int i = 0; COMMAND_KEYS[i].key != 0; i++) {
                if (COMMAND_KEYS[i].alt_key &&
                    strcasecmp(word, COMMAND_KEYS[i].alt_key) == 0) {
                    cmd.type = COMMAND_KEYS[i].type;
                    break;
                }
            }
        }

        if (cmd.type == CMD_NONE) {
            cmd.type = CMD_UNKNOWN;
            return cmd;
        }
    }

    /* Parse target arguments */
    const char* args = input + 1;
    while (*args && !isspace(*args)) args++;  /* Skip command word */
    while (*args && isspace(*args)) args++;   /* Skip whitespace */

    if (*args) {
        /* Special handling for draw order command */
        if (cmd.type == CMD_DRAW_ORDER) {
            cmd.draw_order_count = parse_draw_order(args, cmd.draw_order, MAX_DRAW_ORDER);
            return cmd;
        }

        /* Special handling for "b w" (buy wanderer) */
        if (cmd.type == CMD_BUY && (tolower(*args) == 'w' || tolower(*args) == 'e')) {
            cmd.type = CMD_BUY_WANDERER;
            return cmd;
        }

        int n = sscanf(args, "%d %d", &cmd.target, &cmd.secondary);

        /* Attack with target becomes ATTACK_BASE */
        if (cmd.type == CMD_ATTACK_PLAYER && n >= 1 && cmd.target >= 0) {
            cmd.type = CMD_ATTACK_BASE;
        }
    }

    return cmd;
}
/* }}} */

/* {{{ command_type_to_string
 * Convert command type to human-readable string.
 */
const char* command_type_to_string(CommandType type) {
    switch (type) {
        case CMD_NONE:          return "none";
        case CMD_PLAY:          return "play";
        case CMD_BUY:           return "buy";
        case CMD_BUY_WANDERER:  return "buy wanderer";
        case CMD_ATTACK_PLAYER: return "attack";
        case CMD_ATTACK_BASE:   return "attack base";
        case CMD_SCRAP:         return "scrap";
        case CMD_ACTIVATE:      return "activate";
        case CMD_DRAW_ORDER:    return "draw order";
        case CMD_END_TURN:      return "end turn";
        case CMD_HELP:          return "help";
        case CMD_QUIT:          return "quit";
        case CMD_UNKNOWN:       return "unknown";
        default:                return "???";
    }
}
/* }}} */

/* {{{ command_is_valid
 * Check if a command is valid (known type, appropriate targets).
 */
bool command_is_valid(Command* cmd) {
    if (cmd == NULL) return false;

    switch (cmd->type) {
        case CMD_NONE:
        case CMD_UNKNOWN:
            return false;

        case CMD_PLAY:
        case CMD_BUY:
        case CMD_SCRAP:
        case CMD_ACTIVATE:
        case CMD_ATTACK_BASE:
            /* These require a target */
            return cmd->target >= 0;

        case CMD_DRAW_ORDER:
            /* Requires at least one card in draw order */
            return cmd->draw_order_count > 0;

        case CMD_BUY_WANDERER:
        case CMD_ATTACK_PLAYER:
        case CMD_END_TURN:
        case CMD_HELP:
        case CMD_QUIT:
            /* These don't require targets */
            return true;

        default:
            return false;
    }
}
/* }}} */

/* {{{ terminal_read_command
 * Read a command from the input window with history support.
 * Uses line editing with up/down arrow history navigation.
 */
Command terminal_read_command(TerminalUI* ui, InputHistory* history) {
    static char buffer[INPUT_MAX_LEN];
    int pos = 0;
    int ch;

    if (ui == NULL || ui->input_win == NULL) {
        return command_parse(NULL);
    }

    /* Reset history position for new input */
    if (history) {
        input_history_reset_position(history);
    }

    /* Clear and show prompt */
    werase(ui->input_win);
    wattron(ui->input_win, COLOR_PAIR(COLOR_PAIR_HIGHLIGHT));
    mvwprintw(ui->input_win, 0, 0, " > ");
    wattroff(ui->input_win, COLOR_PAIR(COLOR_PAIR_HIGHLIGHT));
    wrefresh(ui->input_win);

    /* Enable cursor and input */
    curs_set(1);
    echo();
    nodelay(ui->input_win, FALSE);  /* Blocking input */

    buffer[0] = '\0';
    pos = 0;

    while (1) {
        ch = wgetch(ui->input_win);

        if (ch == '\n' || ch == KEY_ENTER) {
            break;
        }

        if (ch == KEY_UP && history) {
            const char* prev = input_history_prev(history);
            if (prev) {
                strncpy(buffer, prev, sizeof(buffer) - 1);
                buffer[sizeof(buffer) - 1] = '\0';
                pos = strlen(buffer);

                /* Redraw input line */
                werase(ui->input_win);
                wattron(ui->input_win, COLOR_PAIR(COLOR_PAIR_HIGHLIGHT));
                mvwprintw(ui->input_win, 0, 0, " > ");
                wattroff(ui->input_win, COLOR_PAIR(COLOR_PAIR_HIGHLIGHT));
                mvwprintw(ui->input_win, 0, 3, "%s", buffer);
                wrefresh(ui->input_win);
            }
            continue;
        }

        if (ch == KEY_DOWN && history) {
            const char* next = input_history_next(history);
            if (next) {
                strncpy(buffer, next, sizeof(buffer) - 1);
                buffer[sizeof(buffer) - 1] = '\0';
            } else {
                buffer[0] = '\0';
            }
            pos = strlen(buffer);

            /* Redraw input line */
            werase(ui->input_win);
            wattron(ui->input_win, COLOR_PAIR(COLOR_PAIR_HIGHLIGHT));
            mvwprintw(ui->input_win, 0, 0, " > ");
            wattroff(ui->input_win, COLOR_PAIR(COLOR_PAIR_HIGHLIGHT));
            mvwprintw(ui->input_win, 0, 3, "%s", buffer);
            wrefresh(ui->input_win);
            continue;
        }

        if ((ch == KEY_BACKSPACE || ch == 127 || ch == 8) && pos > 0) {
            pos--;
            buffer[pos] = '\0';

            /* Redraw input line */
            werase(ui->input_win);
            wattron(ui->input_win, COLOR_PAIR(COLOR_PAIR_HIGHLIGHT));
            mvwprintw(ui->input_win, 0, 0, " > ");
            wattroff(ui->input_win, COLOR_PAIR(COLOR_PAIR_HIGHLIGHT));
            mvwprintw(ui->input_win, 0, 3, "%s", buffer);
            wrefresh(ui->input_win);
            continue;
        }

        if (ch >= 32 && ch < 127 && pos < INPUT_MAX_LEN - 1) {
            buffer[pos++] = ch;
            buffer[pos] = '\0';
        }
    }

    /* Restore cursor state */
    curs_set(0);
    noecho();
    nodelay(ui->input_win, TRUE);

    /* Add to history if non-empty */
    if (buffer[0] != '\0' && history) {
        input_history_add(history, buffer);
    }

    return command_parse(buffer);
}
/* }}} */

/* {{{ terminal_show_prompt
 * Display a prompt in the input window.
 */
void terminal_show_prompt(TerminalUI* ui, const char* prompt) {
    if (ui == NULL || ui->input_win == NULL) return;

    werase(ui->input_win);
    wattron(ui->input_win, COLOR_PAIR(COLOR_PAIR_HIGHLIGHT));
    mvwprintw(ui->input_win, 0, 0, " %s ", prompt ? prompt : ">");
    wattroff(ui->input_win, COLOR_PAIR(COLOR_PAIR_HIGHLIGHT));
    wrefresh(ui->input_win);
}
/* }}} */

/* {{{ input_history_create
 * Create a new input history buffer.
 */
InputHistory* input_history_create(void) {
    InputHistory* history = malloc(sizeof(InputHistory));
    if (history == NULL) return NULL;

    history->count = 0;
    history->position = -1;

    return history;
}
/* }}} */

/* {{{ input_history_free
 * Free an input history buffer.
 */
void input_history_free(InputHistory* history) {
    if (history != NULL) {
        free(history);
    }
}
/* }}} */

/* {{{ input_history_add
 * Add a command to the history buffer.
 */
void input_history_add(InputHistory* history, const char* input) {
    if (history == NULL || input == NULL || input[0] == '\0') return;

    /* Don't add duplicates of the last entry */
    if (history->count > 0 &&
        strcmp(history->entries[history->count - 1], input) == 0) {
        return;
    }

    /* Shift entries if full */
    if (history->count >= INPUT_HISTORY_SIZE) {
        memmove(history->entries[0], history->entries[1],
                (INPUT_HISTORY_SIZE - 1) * INPUT_MAX_LEN);
        history->count = INPUT_HISTORY_SIZE - 1;
    }

    /* Add new entry */
    strncpy(history->entries[history->count], input, INPUT_MAX_LEN - 1);
    history->entries[history->count][INPUT_MAX_LEN - 1] = '\0';
    history->count++;
    history->position = -1;
}
/* }}} */

/* {{{ input_history_prev
 * Navigate to previous history entry.
 */
const char* input_history_prev(InputHistory* history) {
    if (history == NULL || history->count == 0) return NULL;

    if (history->position < 0) {
        /* Start at most recent */
        history->position = history->count - 1;
    } else if (history->position > 0) {
        history->position--;
    } else {
        /* Already at oldest, stay there */
        return history->entries[0];
    }

    return history->entries[history->position];
}
/* }}} */

/* {{{ input_history_next
 * Navigate to next history entry.
 */
const char* input_history_next(InputHistory* history) {
    if (history == NULL || history->position < 0) return NULL;

    if (history->position < history->count - 1) {
        history->position++;
        return history->entries[history->position];
    }

    /* Past end of history, return to new input */
    history->position = -1;
    return NULL;
}
/* }}} */

/* {{{ input_history_reset_position
 * Reset history navigation position.
 */
void input_history_reset_position(InputHistory* history) {
    if (history != NULL) {
        history->position = -1;
    }
}
/* }}} */

/* {{{ terminal_show_help
 * Display command help in the narrative window.
 */
void terminal_show_help(TerminalUI* ui) {
    if (ui == NULL || ui->narrative_win == NULL) return;

    werase(ui->narrative_win);
    box(ui->narrative_win, 0, 0);
    mvwprintw(ui->narrative_win, 0, 2, " COMMANDS ");

    int y = 2;
    wattron(ui->narrative_win, COLOR_PAIR(COLOR_PAIR_TRADE));
    mvwprintw(ui->narrative_win, y++, 2, "p <n>  - Play card n from hand");
    mvwprintw(ui->narrative_win, y++, 2, "b <n>  - Buy card n from trade row");
    wattroff(ui->narrative_win, COLOR_PAIR(COLOR_PAIR_TRADE));

    wattron(ui->narrative_win, COLOR_PAIR(COLOR_PAIR_COMBAT));
    mvwprintw(ui->narrative_win, y++, 2, "a      - Attack opponent");
    mvwprintw(ui->narrative_win, y++, 2, "a <n>  - Attack opponent's base n");
    wattroff(ui->narrative_win, COLOR_PAIR(COLOR_PAIR_COMBAT));

    wattron(ui->narrative_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwprintw(ui->narrative_win, y++, 2, "s <n>  - Scrap card n");
    mvwprintw(ui->narrative_win, y++, 2, "x <n>  - Activate base ability n");
    mvwprintw(ui->narrative_win, y++, 2, "e      - End your turn");
    y++;
    mvwprintw(ui->narrative_win, y++, 2, "h or ? - Show this help");
    mvwprintw(ui->narrative_win, y++, 2, "q      - Quit game");
    wattroff(ui->narrative_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));

    wrefresh(ui->narrative_win);
}
/* }}} */

/* {{{ terminal_show_error
 * Display an error message in the input window.
 */
void terminal_show_error(TerminalUI* ui, const char* message) {
    if (ui == NULL || ui->input_win == NULL) return;

    werase(ui->input_win);
    wattron(ui->input_win, COLOR_PAIR(COLOR_PAIR_ERROR));
    mvwprintw(ui->input_win, 0, 0, " Error: %s ", message ? message : "Unknown error");
    wattroff(ui->input_win, COLOR_PAIR(COLOR_PAIR_ERROR));
    wrefresh(ui->input_win);
}
/* }}} */

/* {{{ terminal_show_message
 * Display a message in the input window.
 */
void terminal_show_message(TerminalUI* ui, const char* message) {
    if (ui == NULL || ui->input_win == NULL) return;

    werase(ui->input_win);
    wattron(ui->input_win, COLOR_PAIR(COLOR_PAIR_SUCCESS));
    mvwprintw(ui->input_win, 0, 0, " %s ", message ? message : "");
    wattroff(ui->input_win, COLOR_PAIR(COLOR_PAIR_SUCCESS));
    wrefresh(ui->input_win);
}
/* }}} */

/* {{{ terminal_show_help_for_phase
 * Display phase-appropriate command help.
 */
void terminal_show_help_for_phase(TerminalUI* ui, GamePhase phase) {
    if (ui == NULL || ui->narrative_win == NULL) return;

    werase(ui->narrative_win);
    box(ui->narrative_win, 0, 0);

    int y = 2;

    switch (phase) {
        case PHASE_DRAW_ORDER:
            mvwprintw(ui->narrative_win, 0, 2, " DRAW ORDER ");
            wattron(ui->narrative_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
            mvwprintw(ui->narrative_win, y++, 2, "Set order to draw cards:");
            mvwprintw(ui->narrative_win, y++, 2, "d 3,1,5,2,4 - Draw in this order");
            y++;
            mvwprintw(ui->narrative_win, y++, 2, "Numbers are card indices from");
            mvwprintw(ui->narrative_win, y++, 2, "your deck (1-based)");
            wattroff(ui->narrative_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
            break;

        case PHASE_WAITING:
            mvwprintw(ui->narrative_win, 0, 2, " WAITING ");
            wattron(ui->narrative_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
            mvwprintw(ui->narrative_win, y++, 2, "Waiting for opponent...");
            y++;
            mvwprintw(ui->narrative_win, y++, 2, "q - Quit game");
            wattroff(ui->narrative_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
            break;

        case PHASE_GAME_OVER:
            mvwprintw(ui->narrative_win, 0, 2, " GAME OVER ");
            wattron(ui->narrative_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
            mvwprintw(ui->narrative_win, y++, 2, "q - Quit to menu");
            wattroff(ui->narrative_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
            break;

        default:
            /* Main phase - show full help */
            terminal_show_help(ui);
            return;
    }

    wrefresh(ui->narrative_win);
}
/* }}} */

/* {{{ command_to_json
 * Convert a command to JSON protocol message.
 * Caller must free the returned string with command_json_free().
 */
char* command_to_json(Command* cmd) {
    if (cmd == NULL) return NULL;

    char* json = malloc(512);
    if (json == NULL) return NULL;

    switch (cmd->type) {
        case CMD_PLAY:
            snprintf(json, 512,
                "{\"type\":\"action\",\"action\":\"play_card\",\"card_index\":%d}",
                cmd->target);
            break;

        case CMD_BUY:
            snprintf(json, 512,
                "{\"type\":\"action\",\"action\":\"buy_card\",\"slot_index\":%d}",
                cmd->target);
            break;

        case CMD_BUY_WANDERER:
            snprintf(json, 512,
                "{\"type\":\"action\",\"action\":\"buy_wanderer\"}");
            break;

        case CMD_ATTACK_PLAYER:
            snprintf(json, 512,
                "{\"type\":\"action\",\"action\":\"attack_player\"}");
            break;

        case CMD_ATTACK_BASE:
            snprintf(json, 512,
                "{\"type\":\"action\",\"action\":\"attack_base\",\"base_index\":%d}",
                cmd->target);
            break;

        case CMD_SCRAP:
            snprintf(json, 512,
                "{\"type\":\"action\",\"action\":\"scrap_card\",\"card_index\":%d}",
                cmd->target);
            break;

        case CMD_ACTIVATE:
            snprintf(json, 512,
                "{\"type\":\"action\",\"action\":\"activate_base\",\"base_index\":%d}",
                cmd->target);
            break;

        case CMD_DRAW_ORDER: {
            int off = snprintf(json, 512,
                "{\"type\":\"action\",\"action\":\"set_draw_order\",\"order\":[");
            for (int i = 0; i < cmd->draw_order_count && off < 500; i++) {
                if (i > 0) off += snprintf(json + off, 512 - off, ",");
                off += snprintf(json + off, 512 - off, "%d", cmd->draw_order[i]);
            }
            snprintf(json + off, 512 - off, "]}");
            break;
        }

        case CMD_END_TURN:
            snprintf(json, 512,
                "{\"type\":\"action\",\"action\":\"end_turn\"}");
            break;

        default:
            free(json);
            return NULL;
    }

    return json;
}
/* }}} */

/* {{{ command_json_free
 * Free a JSON string allocated by command_to_json.
 */
void command_json_free(char* json) {
    if (json != NULL) {
        free(json);
    }
}
/* }}} */

/* {{{ command_valid_for_phase
 * Check if a command is valid for the current game phase.
 */
bool command_valid_for_phase(Command* cmd, GamePhase phase) {
    if (cmd == NULL) return false;

    /* Help and quit always valid */
    if (cmd->type == CMD_HELP || cmd->type == CMD_QUIT) {
        return true;
    }

    switch (phase) {
        case PHASE_MAIN:
            /* All game commands valid in main phase */
            return cmd->type == CMD_PLAY || cmd->type == CMD_BUY ||
                   cmd->type == CMD_BUY_WANDERER || cmd->type == CMD_ATTACK_PLAYER ||
                   cmd->type == CMD_ATTACK_BASE || cmd->type == CMD_SCRAP ||
                   cmd->type == CMD_ACTIVATE || cmd->type == CMD_END_TURN;

        case PHASE_DRAW_ORDER:
            /* Only draw order command valid */
            return cmd->type == CMD_DRAW_ORDER;

        case PHASE_WAITING:
        case PHASE_GAME_OVER:
            /* Only help/quit valid (already checked above) */
            return false;

        default:
            return false;
    }
}
/* }}} */

/* {{{ command_validation_error
 * Get error message for why a command is invalid.
 */
const char* command_validation_error(Command* cmd, GamePhase phase) {
    if (cmd == NULL) return "No command";

    if (cmd->type == CMD_UNKNOWN) {
        return "Unknown command. Type 'h' for help.";
    }

    if (cmd->type == CMD_NONE) {
        return "Empty command";
    }

    /* Check if valid for phase */
    if (!command_valid_for_phase(cmd, phase)) {
        switch (phase) {
            case PHASE_DRAW_ORDER:
                return "Use 'd' command to set draw order";
            case PHASE_WAITING:
                return "Waiting for opponent";
            case PHASE_GAME_OVER:
                return "Game is over";
            default:
                return "Invalid command for this phase";
        }
    }

    /* Check target requirements */
    if (!command_is_valid(cmd)) {
        switch (cmd->type) {
            case CMD_PLAY:
                return "Usage: p <card_index>";
            case CMD_BUY:
                return "Usage: b <slot_index> or 'b w' for wanderer";
            case CMD_ATTACK_BASE:
                return "Usage: a <base_index>";
            case CMD_SCRAP:
                return "Usage: s <card_index>";
            case CMD_ACTIVATE:
                return "Usage: x <base_index>";
            case CMD_DRAW_ORDER:
                return "Usage: d 3,1,5,2,4";
            default:
                return "Missing required argument";
        }
    }

    return NULL;  /* No error */
}
/* }}} */

/* {{{ command_complete
 * Get completion for partial command.
 * Returns the completed command or NULL if no match.
 * match_count is set to number of matches found.
 */
const char* command_complete(const char* partial, int* match_count) {
    if (partial == NULL || partial[0] == '\0') {
        if (match_count) *match_count = 0;
        return NULL;
    }

    int len = strlen(partial);
    const char* match = NULL;
    int count = 0;

    for (int i = 0; COMMAND_NAMES[i] != NULL; i++) {
        if (strncasecmp(COMMAND_NAMES[i], partial, len) == 0) {
            if (match == NULL) match = COMMAND_NAMES[i];
            count++;
        }
    }

    if (match_count) *match_count = count;
    return (count == 1) ? match : NULL;
}
/* }}} */

/* {{{ terminal_show_completions
 * Show available command completions in input window.
 */
void terminal_show_completions(TerminalUI* ui, const char* partial) {
    if (ui == NULL || ui->input_win == NULL) return;

    int len = partial ? strlen(partial) : 0;
    char matches[256] = "";
    int off = 0;

    for (int i = 0; COMMAND_NAMES[i] != NULL && off < 240; i++) {
        if (len == 0 || strncasecmp(COMMAND_NAMES[i], partial, len) == 0) {
            if (off > 0) off += snprintf(matches + off, 256 - off, " ");
            off += snprintf(matches + off, 256 - off, "%s", COMMAND_NAMES[i]);
        }
    }

    werase(ui->input_win);
    wattron(ui->input_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
    mvwprintw(ui->input_win, 0, 0, " Commands: %s ", matches);
    wattroff(ui->input_win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
    wrefresh(ui->input_win);
}
/* }}} */
