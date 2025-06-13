/// ****************************************************************************
/// @file tr_cli.h
///
/// @brief common CLI utility
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_CLI_H
#define TR_CLI_H

#include <stdbool.h>
#include "tr_printf.h"
#include "tr_cli_command_parser.h"
#include "tr_cli_buffer.h"

#ifndef TR_CLI_MAX_LINE
/**
 * Maximum number of bytes to accept in a single line
 */
#define TR_CLI_MAX_LINE 120
#endif

#ifndef TR_CLI_HISTORY_LEN
/**
 * Maximum number of bytes to retain of history data
 * Define this to 0 to remove history support
 */
#define TR_CLI_HISTORY_LEN 1000
#endif

#ifndef TR_CLI_MAX_ARGC
/**
 * What is the maximum number of arguments we reserve space for
 */
#define TR_CLI_MAX_ARGC 16
#endif

#ifndef TR_CLI_PROMPT
/**
 * CLI prompt to display after pressing enter
 */
#define TR_CLI_PROMPT "trident> "
#endif

#ifndef TR_CLI_MAX_PROMPT_LEN
/**
 * Maximum number of bytes in the prompt
 */
#define TR_CLI_MAX_PROMPT_LEN 10
#endif

#ifndef TR_CLI_SERIAL_XLATE
/**
 * Translate CR -> NL on input and output CR NL on output. This allows
 * "natural" processing when using a serial terminal.
 */
#define TR_CLI_SERIAL_XLATE 1
#endif

#ifndef TR_CLI_LOCAL_ECHO
/**
 * Enable or disable local echo
 */
#define TR_CLI_LOCAL_ECHO 1
#endif

/**
 * This is the structure which defines the current state of the CLI
 * NOTE: Although this structure is exposed here, it is not recommended
 * that it be interacted with directly. Use the accessor functions below to
 * interact with it. It is exposed here to make it easier to use as a static
 * structure, but all elements of the structure should be considered private
 */
struct tr_cli
{
    /**
     * Internal buffer. This should not be accessed directly, use the
     * access functions below
     */
    char buffer[TR_CLI_MAX_LINE];

#if TR_CLI_HISTORY_LEN
    /**
     * List of history entries
     */
    char history[TR_CLI_HISTORY_LEN];

    /**
     * Are we searching through the history?
     */
    bool searching;

    /**
     * How far back in the history are we?
     */
    int history_pos;
#endif

    /**
     * Number of characters in buffer at the moment
     */
    int len;

    /**
     * Position of the cursor
     */
    int cursor;

    /**
     * Have we just parsed a full line?
     */
    bool done;

    /**
     * Callback function to output a single character to the user
     */
    void (*put_char)(char ch);

    /**
     * Data to provide to the put_char callback
     */
    void *cb_data;

    bool have_escape;
    bool have_csi;

    /**
     * counter of the value for the CSI code
     */
    int counter;

    char *argv[TR_CLI_MAX_ARGC];

    char prompt[TR_CLI_MAX_PROMPT_LEN];
};

/**
 * Start up the Embedded CLI subsystem. This should only be called once.
 */
void tr_cli_init(const char *prompt,
                 void (     *put_char )(char ch));

/**
 * Adds a new character into the buffer. Returns true if
 * the buffer should now be processed
 * Note: This function should not be called from an interrupt handler.
 */
bool tr_cli_insert_char(char ch);

/**
 * Returns the nul terminated internal buffer. This will
 * return NULL if the buffer is not yet complete
 */
const char *tr_cli_get_line(void);

/**
 * Parses the internal buffer and returns it as an argc/argc combo
 * @return number of values in argv (maximum of TR_CLI_MAX_ARGC)
 */
int tr_cli_argc(char ***argv);

/**
 * Outputs the CLI prompt
 * This should be called after @ref tr_cli_argc or @ref
 * tr_cli_get_line has been called and the command fully processed
 */
void tr_cli_prompt(void);

/**
 * Retrieve a history command line
 * @param history_pos 0 is the most recent command, 1 is the one before that
 * etc...
 * @return NULL if the history buffer is exceeded
 */
const char *tr_cli_get_history(int history_pos);

/**
 * Pass a received character to the cli
 * @param data character received on the terminal
 */
void tr_cli_char_received(char data);

/**
 * printf function used for printing to terminal
 * This function can be re-defined by the application using the cli
 * to add additional formatting behavior if desired.
 * @param pFormat printf format string
 * @param ... printf parameters
 */
void tr_cli_common_printf(const char *pFormat,
                          ...);

#endif // TR_CLI_H
