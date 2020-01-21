// History, undo, redo. Free and open source. See LICENSE.
// TODO: Elide successive 1-char insertions.
#include "history.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// A history structure consists of a flexible array of bytes, with a current
// position in the history during undo/redo sequences.
struct history { int current, length, max; char *bs; };

history *newHistory() {
    history *h = malloc(sizeof(history));
    *h = (history) { .current=0, .length=0, .max=1000, .bs=malloc(1000) };
    return h;
}

void freeHistory(history *h) {
    free(h->bs);
    free(h);
}

// Resize the byte array to fit n more bytes.
static void resize(history *h, int n) {
    while (h->length + n > h->max) h->max = h->max * 3 / 2;
    h->bs = realloc(h->bs, h->max);
}

// Add a byte to the history.
static inline void save(history *h, char b) {
    if (h->length + 1 > h->max) resize(h, 1);
    h->bs[h->length++] = b;
    h->current = h->length;
}

// Add n bytes to the history.
static inline void saveString(history *h, int n, char const *s) {
    if (h->length + n > h->max) resize(h, n);
    memcpy(&h->bs[h->length], s, n);
    h->length += n;
    h->current = h->length;
}

// Check whether a byte is illegal, within valid UTF-8 text (ignoring 0x00, 0xC0
// and 0xC1).
static inline bool illegal(unsigned char b) {
    return b >= 0xF5;
}

// An insert or delete is stored as "..." OP and other operations as N OP.
// Putting the OP after the argument makes adding the 'end' flag easier. An
// opcode is made negative and shifted left one bit to make room for the 'end'
// flag. All opcode bytes have the top bit set to distinguish them from
// numerical arguments, and the Move/Insert/Delete opcode bytes are illegal in
// UTF-8 text so they can be used as terminators at either end of strings.
static inline void saveOp(history *h, unsigned op) {
    save(h, (0xFF - op) << 1);
}

// Add a signed integer argument to the history, packed in bytes with the top
// bit zero. The opcodes on either side delimit it. If there are no argument
// bytes, the argument is zero or not needed. (Avoid relying on arithmetic right
// shift of negative integers.)
static void saveInt(history *h, int n) {
    if (n == 0) return;
    if (n < -134217728 || n >= 134217728) save(h, (n >> 28) & 0x7F);
    if (n < -1048576 || n >= 1048576) save(h, (n >> 21) & 0x7F);
    if (n < -8192 || n >= 8192) save(h, (n >> 14) & 0x7F);
    if (n < -64 || n >= 64) save(h, (n >> 7) & 0x7F);
    save(h, n & 0x7F);
}

// Save an opcode and integer argument (in reverse order). If this is after an
// undo/redo sequence, truncate the history first.
static void saveOpN(history *h, int op, int by) {
    if (h->length > h->current) h->length = h->current;
    saveInt(h,by);
    saveOp(h,op);
    h->current = h->length;
}

// Save an opcode and a string (in reverse order). Must come after a Move,
// so is not immediately after an undo/redo sequence.
static void saveOpS(history *h, int op, int n, char const *s) {
    saveString(h,n,s);
    saveOp(h,op);
    h->current = h->length;
}

void saveMove(history *h, int n) { saveOpN(h, Move, n); }
void saveInsert(history *h, int p, int n, char const *s) {
    saveMove(h, p);
    saveOpS(h, Insert, n, s);
}
void saveDelete(history *h, int p, int n, char const *s) {
    saveMove(h, p);
    saveOpS(h, Delete, n, s);
}
void saveAddCursor(history *h, int n) { saveOpN(h, AddCursor, n); }
void saveCutCursor(history *h, int n) { saveOpN(h, CutCursor, n); }
void saveSetCursor(history *h, int n) { saveOpN(h, SetCursor, n); }
void saveCursorRow(history *h, int n) { saveOpN(h, CursorRow, n); }
void saveCursorCol(history *h, int n) { saveOpN(h, CursorCol, n); }
void saveBaseRow(history *h, int n) { saveOpN(h, BaseRow, n); }
void saveBaseCol(history *h, int n) { saveOpN(h, BaseCol, n); }
void saveMarkRow(history *h, int n) { saveOpN(h, MarkRow, n); }
void saveMarkCol(history *h, int n) { saveOpN(h, MarkCol, n); }
void saveEnd(history *h) { if (h->length > 0) h->bs[h->length-1] |= 1; }

// Extract the opcode from a byte.
static int getOp(unsigned char b) {
    return 0xFF - ((b >> 1) | 0x80);
}

// Extract the 'end' flag from a byte.
static bool getEnd(unsigned char b) {
    return (b & 1) != 0;
}

// Unpack an int from a range of bytes with the top bit unset. Avoid left shift
// of negative numbers.
static int unpack(history *h, int start, int end) {
    if (start == end) return 0;
    char ch = h->bs[start];
    bool neg = (ch & 0x40) != 0;
    unsigned int n = neg ? -1 : 0;
    for (int i = start; i < end; i++) n = (n << 7) | h->bs[i];
    return n;
}

// Pop an op and 'end' flag backward off the history.
static void undoOpEnd(history *h, edit *e) {
    if (h->current == 0) { e->op = End; return; };
    unsigned char b = h->bs[--h->current];
    e->op = getOp(b);
    e->end = getEnd(b);
}

// Pop an integer backward off the history.
static void undoInt(history *h, edit *e) {
    int end = h->current, start;
    for (start = end; start > 0 && (h->bs[start-1] & 0x80) == 0; start--) {}
    h->current = start;
    e->n = unpack(h, start, end);
}

// Pop a string backward off the history.
static void undoString(history *h, edit *e) {
    int i;
    for (i = h->current; i > 0 && ! illegal(h->bs[i-1]); i--) { }
    e->n = h->current - i;
    h->current = i;
    e->s = &h->bs[h->current];
}

// Invert an edit.
static void invert(edit *e) {
    switch (e->op) {
        case Insert: e->op = Delete; break;
        case Delete: e->op = Insert; break;
        case AddCursor: e->op = CutCursor; break;
        case CutCursor: e->op = AddCursor; break;
        default: e->n = - e->n; break;
    }
}

edit undo(history *h) {
    edit e = { .end=false, .op=0, .n=0, .s=NULL };
    undoOpEnd(h, &e);
    if (e.op == Insert || e.op == Delete) undoString(h, &e);
    else undoInt(h, &e);
    invert(&e);
    return e;
}

// ----------------------------------------------------------------------------
/*
// Get the most recent op.
static int endOp(history *h) {
    if (h->current == 0) return End;
    return getOp(h->bs[h->current-1]);
}

// Read an opcode forward off the history (for redo).
static int readOp(history *h) {
    if (h->current == h->length) return End;
    return getOp(h->bs[h->current++]);
}

// Read an integer forward off the history (for redo).
static int readInt(history *h) {
    int start = h->current, end;
    for (end = start; end < h->length && (h->bs[end] & 0x80) == 0; end++) {}
    h->current = end;
    return unpack(h, start, end);
}

// Read a string from the history, returning the number of bytes in *pn.
static char const *readString(history *h, int *pn) {
    int i;
    for (i = h->current; i < h->length && h->bs[i] < firstIllegal; i++) { }
    *pn = i - h->current;
    int old = h->current;
    h->current = i;
    return &h->bs[old];
}

// Read an edit forwards from the history, for redo.
static edit readEdit(history *h) {
    edit e = { .end=false, .op=End, .n=0, .s=NULL };
    if (h->current >= h->length) return e;
    if (endOp(h) == Move) {
        e.s = readString(h, &e.n);
        e.op = readOp(h);
    }
    else {
        e.n = readInt(h);
        e.op = readOp(h);
    }
    char ch = h->bs[h->current++];
    e.end = ch & 1;
    e.op = ch >> 1;
    return e;
}

edit undo(history *h) {
    edit e = { .op = End };
    if (h->current == 0) return e;
    e = popEdit(h);
    invert(&e);
    return e;
}

edit redo(history *h) {
    edit e = { .op = End };
    if (h->current == h->length) return e;
    return readEdit(h);
}
*/
#ifdef historyTest
// ----------------------------------------------------------------------------

// Check that Move, Insert and Delete ops can't clash with text bytes.
static void testOps() {
    assert(Move <= 2 && Insert <= 2 && Delete <= 2);
}

// Check that an integer can be saved and popped.
static bool checkInt(history *h, int n) {
    edit e = { .end=false, .op=0, .n=0, .s=NULL };
    h->current = h->length = 0;
    saveInt(h, n);
    undoInt(h, &e);
    return (h->current == 0 && e.n == n);
}

// Check that integers around the boundaries can be saved and popped.
static void testInts(history *h) {
    assert(checkInt(h,0));
    assert(checkInt(h,1));
    assert(checkInt(h,63));
    assert(checkInt(h,64));
    assert(checkInt(h,8191));
    assert(checkInt(h,8192));
    assert(checkInt(h,1048575));
    assert(checkInt(h,1048576));
    assert(checkInt(h,134217727));
    assert(checkInt(h,134217728));
    assert(checkInt(h,2147483647));
    assert(checkInt(h,-1));
    assert(checkInt(h,-64));
    assert(checkInt(h,-65));
    assert(checkInt(h,-8192));
    assert(checkInt(h,-8193));
    assert(checkInt(h,-1048576));
    assert(checkInt(h,-1048577));
    assert(checkInt(h,-134217728));
    assert(checkInt(h,-134217729));
    assert(checkInt(h,-2147483648));
}

// Check that an edit can be saved and undone.
static bool checkUndo(history *h, int op, int n, char *s) {
    edit e = { .op=op, .n=n, .s=s };
    h->current = h->length = 0;
    saveOpN(h, Move, 0);
    int start = h->current;
    if (s == NULL) saveOpN(h, op, n);
    else saveOpS(h, op, n, s);
    edit e2 = undo(h);
    invert(&e);
    return
        h->current == start && e2.op==e.op && e2.n==e.n &&
        (e.s == NULL || strncmp(e2.s, e.s, e.n) == 0);
}

// Check that edits can be saved and undone.
static void testUndo(history *h) {
    assert(checkUndo(h, Move, 42, NULL));
    assert(checkUndo(h, Insert, 3, "abc"));
    assert(checkUndo(h, Delete, 4, "wxyz"));
    assert(checkUndo(h, AddCursor, 100, NULL));
    assert(checkUndo(h, CutCursor, 100, NULL));
    assert(checkUndo(h, SetCursor, 4, NULL));
    assert(checkUndo(h, CursorRow, 100, NULL));
}

int main() {
    setbuf(stdout, NULL);
    testOps();
    history *h = newHistory();
    testInts(h);
    testUndo(h);
    freeHistory(h);
    printf("History module OK\n");
    return 0;
}

#endif
