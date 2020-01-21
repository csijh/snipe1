// History, undo, redo. Free and open source. See LICENSE.
#include <stdbool.h>

// A history object records insertions, deletions, and cursor changes, for undo
// or redo. Each user action becomes a sequence of such edits, including
// automatic adjustments such as re-indenting. The edits and their restrictions
// are designed so that the edits are invertible.
struct history;
typedef struct history history;

// Remove all the entries.
void clearHistory(history *h);

// Save a change of insert/delete position, relative to the previous one,
// then an insertion of a string s of length n.
void saveInsert(history *h, int p, int n, char const *s);

// Save a change of insert/delete position, relative to the previous one,
// then a deletion of a string s of length n before that position.
void saveDelete(history *h, int p, int n, char const *s);

// Save an addition of a new cursor at the given relative index, at the same
// point as the current cursor (or previous cursor, if new index is #cursors).
void saveAddCursor(history *h, int n);

// Save a deletion of the current cursor, making the given relative index
// current. The cursor must already have been moved to the same point as the
// next (or previous) cursor.
void saveCutCursor(history *h, int n);

// Save a relative change of current cursor index.
void saveSetCursor(history *h, int n);

// Save a relative movement of the current cursor row. The cursor doesn't have a
// selection and the order of the cursors is preserved.
void saveCursorRow(history *h, int n);

// Save a relative movement of the current cursor column. The cursor doesn't
// have a selection and the order of the cursors is preserved.
void saveCursorCol(history *h, int n);

// Save a relative movement of the row of the base of the current cursor,
// independent of the mark, preserving the order of the cursors.
void saveBaseRow(history *h, int n);

// Save a relative movement of the column of the base of the current cursor,
// independent of the mark, preserving the order of the cursors.
void saveBaseCol(history *h, int n);

// Save a relative movement of the row of the mark of the current cursor.
void saveMarkRow(history *h, int n);

// Save a relative movement of the column of the mark of the current cursor.
void saveMarkCol(history *h, int n);

// Record the end of the current user action.
void saveEnd(history *h);

// An edit, retrieved for undo or redo. The string s, if any, is only valid
// until the next edit. The 'end' flag means the edit is the last of an undo or
// redo sequence corresponding to a user action.
struct edit { bool end; int op, n; char const *s; };
typedef struct edit edit;

// Edits as opcodes. A Move is a change of relative insert/delete position,
// which precedes the Insert or Delete itself.
enum op {
    Move, Insert, Delete, AddCursor, CutCursor, SetCursor, CursorRow, CursorCol,
    BaseRow, BaseCol, MarkRow, MarkCol, End
};

// Get the most recent edit, inverted ready to execute. This should be repeated
// until the 'last' flag is set. If the opcode is End, there are no edits to
// undo. (Insert and Delete are inverses, AddCursor and CutCursor are inverses,
// and the rest are self-inverses by negation.)
edit undo(history *h);

// Get the most recent undone action, ready for re-execution. This should be
// repeated until the last flag is set.
edit redo(history *h);
