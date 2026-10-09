#ifndef CLASSICAL_HISTORY_H
#define CLASSICAL_HISTORY_H

/*
 * Chat history for the gateway conversation.
 *
 * Logs are stored as a plain text file
 * <exedir>\chatlog\chatlog.txt
 * so that the conversation persists
 * across restarts.
 */


/*
 * Append a user/AI exchange to the log.
 */
void HistoryAppend(
    const char *userText,
    const char *aiText
);


/*
 * Return the full path to the log file.
 *
 * The returned pointer references an
 * internal static buffer that is
 * overwritten on the next call.
 */
const char *HistoryFilePath(
    void
);


/*
 * Return TRUE if the log file exists
 * on disk.
 */
int HistoryExists(
    void
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