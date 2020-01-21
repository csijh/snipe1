// Cursors and selections. Free and open source. See LICENSE.
// TODO: fully implement multiple cursors.
#include "history.h"

// A cursor has a base and a mark, which are equal if there is no selection. Row
// and column coordinates (zero-based) are used, so a cursor can be beyond the
// physical end of its line, or beyond the physical last line. There is a
// remembered column for up/down movement. The cursors are kept in order of
// position of their bases in the text, with one cursor being current. After a
// user action, cursors are merged so that they don't overlap, and they only
// touch if there is no visual ambiguity, i.e. they both have selections and it
// is not the two bases which touch. During drag actions, the cursors are not
// merged until the drop action.
struct cursors;
typedef struct cursors cursors;

// Allocate cursors with access to history, initially with one cursor at (0,0).
cursors *newCursors(history *h);

// Free the cursors, but not the history.
void freeCursors(cursors *cs);

// Return the number of cursors.
int nCursors(cursors *cs);

// Return the index of the current cursor.
int currentCursor(cursors *cs);

// Set the i'th cursor as current.
void setCursor(cursors *cs, int i);

// Move the current cursor.
void moveCursor(cursors *cs, int row, int col);

// Add a cursor at the given point, at the right index. It becomes current.
void addCursor(cursors *cs, int row, int col);

// Move the current cursor's base.
void baseCursor(cursors *cs, int row, int col);

// Move the current cursor's mark.
void markCursor(cursors *cs, int row, int col);

// Merge any overlapping cursors.
void mergeCursor(cursors *cs);

// Get the base and mark, or left and right ends, or remembered column, of the
// current cursor.
int cursorBaseRow(cursors *cs);
int cursorBaseCol(cursors *cs);
int cursorMarkRow(cursors *cs);
int cursorMarkCol(cursors *cs);
int cursorLeftRow(cursors *cs);
int cursorLeftCol(cursors *cs);
int cursorRightRow(cursors *cs);
int cursorRightCol(cursors *cs);
int cursorOldCol(cursors *cs);
