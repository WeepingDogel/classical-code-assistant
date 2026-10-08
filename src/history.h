#ifndef CLASSICAL_HISTORY_H
#define CLASSICAL_HISTORY_H

/*
 * Per-provider chat history.
 *
 * Logs are stored as plain text files in
 * <exedir>\chatlog\Provider<index>.txt
 * so that each provider keeps its own
 * conversation across restarts.
 */


/*
 * Append a user/AI exchange to the log for
 * the given provider index.
 */
void HistoryAppend(
    int providerIndex,
    const char *userText,
    const char *aiText
);


/*
 * Return the full path to the log file
 * for a provider index.
 *
 * The returned pointer references an
 * internal static buffer that is
 * overwritten on the next call.
 */
const char *HistoryFileFor(
    int providerIndex
);


/*
 * Return TRUE if the log file for this
 * provider exists on disk.
 */
int HistoryExists(
    int providerIndex
);


/*
 * Load a log file into a caller-supplied
 * buffer.  Up to maxBytes characters
 * are read (NUL-terminator not
 * included in the count).
 *
 * Returns TRUE on success, FALSE on
 * failure.
 */
int HistoryLoad(
    const char *path,
    char *buffer,
    int maxBytes
);


/*
 * Return TRUE if the chatlog directory
 * exists (or can be created).
 */
int HistoryEnsureDir(void);


#endif