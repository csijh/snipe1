// The Snipe editor is free and open source, see licence.txt.
#include <stdbool.h>
#include "lines.h"
#include "cursors.h"
#include "history.h"

// A text object holds the UTF-8 content of a file. For n bytes, there are n+1
// positions in the text, running from 0 (at the start) to n (after the final
// newline). The text never contains invalid UTF-8 sequences, or control
// characters '\0' to '\7', or tabs, or carriage returns, or trailers (i.e.
// trailing spaces at the ends of lines, or trailing blank lines at the end of
// the file, or a missing final newline). The object supports inserts and
// deletions, and may generate further insertions or deletions as repairs, to
// fix trailers.
struct text;
typedef struct text text;

// Create an empty text object.
text *newText(lines *ls, cursors *cs, history *h);

// Free a text object and its data (but not the objects it is linked to).
void freeText(text *t);

// Return the number of bytes. The maximum length is INT_MAX, so that int can be
// used for positions, and for positive or negative relative positions.
int lengthText(text *t);

// Fill a text object from a newly loaded file, discarding any previous content.
// Return false if the buffer contains invalid UTF-8 sequences or nulls (because
// it is probably binary and shouldn't be loaded).
bool loadText(text *t, int n, char *buffer);

// Copy the text out into a buffer, which must be big enough.
char *saveText(text *t, char *buffer);

// Insert s at a given position. Cursors are adjusted. Any cursor end at the
// given position is moved to the end of the inserted text.
void insertText(text *t, int at, int n, char s[n]);

// Delete text in a range (with to<from allowed). Any cursor end within the
// range, or at the left end, is moved to the right end before the deletion.
void deleteText(text *t, int from, int to);

// Undo the most recent user action, taking normal or small steps. Small steps
// correspond to 'tree-based undo', and also unpick combined typed characters.
void undoText(text *t, bool small);

// Redo the most recent undone user action, taking normal or small steps.
void redoText(text *t, bool small);

// After each edit is executed, the range of text which has been changed can be
// used for incremental changes in other modules, and reset afterwards.
int startChanged(text *t);
int endChanged(text *t);
void resetChanged(text *t);

// Make a copy in s of n characters of text at a given position.
void getText(text *t, int at, int n, char *s);
