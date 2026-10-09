#ifndef CLASSICAL_JSON_H
#define CLASSICAL_JSON_H


/*
 * Build a JSON request body from the user
 * message.
 *
 * The resulting string is a JSON object:
 *   {"message":"<userMessage>"}
 *
 * Special characters (backslash, quote,
 * newline, carriage return, tab) are
 * escaped so that the JSON remains valid.
 *
 * Returns TRUE on success, FALSE if the
 * output buffer is too small.
 */
int JsonBuildRequest(
    const char *userMessage,
    char *outBuf,
    int outSize
);


/*
 * Extract the "response" string from a
 * gateway JSON response.
 *
 * Expected format:
 *   {"response":"<AI reply>"}
 *
 * Unescapes JSON escape sequences.
 *
 * Returns TRUE on success, FALSE if the
 * field is missing or the buffer is too
 * small.
 */
int JsonExtractResponse(
    const char *json,
    char *outBuf,
    int outSize
);


#endif