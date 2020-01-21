// Cursors and selections. Free and open source. See LICENSE.
#include "cursors.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

struct point { int row, col; };
typedef struct point point;

struct cursor { point base, mark; int oldCol; };
typedef struct cursor cursor;

// Multiple cursors are held in a flexible array of structures.
struct cursors {
    int current, length, max;
    cursor *a;
    history *h;
};

cursors *newCursors(history *h) {
    int n = 1;
    cursors *cs = malloc(sizeof(cursors));
    cursor *a = malloc(n * sizeof(cursor));
    *cs = (cursors) { .current=0, .length=1, .max=n, .a=a, .h=h };
    cs->a[0] = (cursor) { .base={0,0}, .mark={0,0}, .oldCol=0 };
    return cs;
}

// Double the capacity.
static void expand(cursors *cs) {
    cs->max = cs->max * 2;
    cs->a = realloc(cs->a, cs->max);
}

void freeCursors(cursors *cs) {
    free(cs->a);
    free(cs);
}

int nCursors(cursors *cs) {
    return cs->length;
}

int currentCursor(cursors *cs) {
    return cs->current;
}

// Primitive operations which include calls to save them in the history.

void setCursor(cursors *cs, int i) {
    if (i < 0) i = 0; else if (i >= cs->length) i = cs->length - 1;
    if (i == cs->current) return;
    saveSetCursor(cs->h, i - cs->current);
    cs->current = i;
}

// Set the base row of the current cursor.
static void setBaseRow(cursors *cs, int r) {
    int old = cs->a[cs->current].base.row;
    if (r == old) return;
    saveBaseRow(cs->h, r - old);
    cs->a[cs->current].base.row = r;
}

// Set the base col of the current cursor.
static void setBaseCol(cursors *cs, int c) {
    int old = cs->a[cs->current].base.col;
    if (c == old) return;
    saveBaseCol(cs->h, c - old);
    cs->a[cs->current].base.col = c;
}

// Set the mark row of the current cursor.
static void setMarkRow(cursors *cs, int r) {
    int old = cs->a[cs->current].mark.row;
    if (r == old) return;
    saveMarkRow(cs->h, r - old);
    cs->a[cs->current].mark.row = r;
}

// Set the mark col of the current cursor.
static void setMarkCol(cursors *cs, int c) {
    int old = cs->a[cs->current].mark.col;
    if (c == old) return;
    saveMarkCol(cs->h, c - old);
    cs->a[cs->current].mark.col = c;
}

// Combinations

// Set the base of the current cursor.
static void setBase(cursors *cs, point p) {
    setBaseRow(cs, p.row);
    setBaseCol(cs, p.col);
}

// Set the mark of the current cursor.
static void setMark(cursors *cs, point p) {
    setMarkRow(cs, p.row);
    setMarkCol(cs, p.col);
}

// Set the current cursor equal to another.
static void setEqual(cursors *cs, int i) {
    setBase(cs, cs->a[i].base);
    setMark(cs, cs->a[i].mark);
    cs->a[cs->current].oldCol = 0;
}

void moveCursor(cursors *cs, int row, int col) {
    point p = { .row=row, .col=col };
    setBase(cs, p);
    setMark(cs, p);
}

void baseCursor(cursors *cs, int row, int col) {
    point p = { .row=row, .col=col };
    setBase(cs, p);
}

void markCursor(cursors *cs, int row, int col) {
    point p = { .row=row, .col=col };
    setMark(cs, p);
}

// Compare one row/col point to another.
static int compare(point p0, point p1) {
    if (p0.row < p1.row) return -1;
    if (p0.row > p1.row) return +1;
    if (p0.col < p1.col) return -1;
    if (p0.col > p1.col) return +1;
    return 0;
}

// Find the right index at which to insert a new cursor, assuming the cursors
// are in a normalised state.
static int find(cursors *cs, int row, int col) {
    point p = { .row=row, .col=col };
    int n = -1;
    for (int i = 0; i < cs->length && n < 0; i++) {
        int pb = compare(p, cs->a[cs->current].base);
        int pm = compare(p, cs->a[cs->current].mark);
        if (pb >= 0 || pm >= 0) n = i;
    }
    if (n < 0) n = cs->length;
    return n;
}

// Set the new cursor initially equal to the next (or prev) cursor then move it.
void addCursor(cursors *cs, int row, int col) {
    if (cs->length >= cs->max) expand(cs);
    int newIndex = find(cs, row, col);
    saveAddCursor(cs->h, newIndex - cs->current);
    setCursor(cs, newIndex);
    for (int i = cs->length - 1; i > cs->current; i--) cs->a[i+1] = cs->a[i];
    cs->length++;
    if (cs->current < cs->length-1) setEqual(cs, cs->current + 1);
    else setEqual(cs, cs->current - 1);
    moveCursor(cs, row, col);
}

// Set cursor equal to next (or prev) before deleting it, to allow undo.
// If last cursor is deleted, should be followed by setCursor.
static void cutCursor(cursors *cs) {
    if (cs->current < cs->length-1) setEqual(cs, cs->current + 1);
    else setEqual(cs, cs->current - 1);
    saveCutCursor(cs->h, 0);
    for (int i = cs->current + 2; i < cs->length; i++) cs->a[i-1] = cs->a[i];
    cs->length--;
}

// Check whether two cursors overlap, including touching ambiguously, i.e. two
// or more of the four points touch or the bases touch.
static bool overlap(cursor *c0, cursor *c1) {
    int bb = compare(c0->base, c1->base);
    int bm = compare(c0->base, c1->mark);
    int mb = compare(c0->mark, c1->base);
    int mm = compare(c0->mark, c1->mark);
    if (bb > 0 || bm > 0 || mb > 0 || mm > 0) return true;
    if (bb < 0 && bm < 0 && mb < 0 && mm < 0) return false;
    if (bb + bm + mb + mm >= -2) return true;
    if (bb == 0) return true;
    return false;
}

// Find the smallest of 4 row/col points.
static point smallest(point p0, point p1, point p2, point p3) {
    point p = p0;
    if (compare(p1, p) < 0) p = p1;
    if (compare(p2, p) < 0) p = p2;
    if (compare(p3, p) < 0) p = p3;
    return p;
}

// Find the largest of 4 row/col points.
static point largest(point p0, point p1, point p2, point p3) {
    point p = p0;
    if (compare(p1, p) > 0) p = p1;
    if (compare(p2, p) > 0) p = p2;
    if (compare(p3, p) > 0) p = p3;
    return p;
}

// Check whether a cursor runs to the left.
static bool leftward(cursor *c) {
    return compare(c->mark, c->base) < 0;
}

// Merge two overlapping cursors.
static void merge(cursors *cs, int i) {
    cursor *c0 = &cs->a[i], *c1 = &cs->a[i+1];
    point base = smallest(c0->base, c0->mark, c1->base, c1->mark);
    point mark = largest(c0->base, c0->mark, c1->base, c1->mark);
    if (leftward(c0) && leftward(c1)) {
        point temp = base;
        base = mark;
        mark = temp;
    }
    setCursor(cs, i+1);
    setBase(cs, base);
    setMark(cs, mark);
    setCursor(cs, i);
    cutCursor(cs);
}

void mergeCursors(cursors *cs) {
    for (int i = 0; i < cs->length - 1; i++) {
        if (overlap(&cs->a[i], &cs->a[i+1])) {
            merge(cs, i);
        }
    }
}

// TODO: extract info functions
// TODO: adjust after insert/delete text
// Convert point to position (at EOL or EOT)
// If position before ins/del, ignore.
// If after, (a) increase/decrease row by newlines.
// (b) if on same line as last line of ins/del, adjust col.

/*
// --------------------
int cursorBase(cursors *cs) {
    return cs->a[cs->current].base;
}

int cursorMark(cursors *cs) {
    return cs->a[cs->current].mark;
}

int cursorLeft(cursors *cs) {
    cursor *c = &cs->a[cs->current];
    if (c->base <= c->mark) return c->base;
    return c->mark;
}

int cursorRight(cursors *cs) {
    cursor *c = &cs->a[cs->current];
    if (c->base >= c->mark) return c->base;
    return c->mark;
}

int cursorCol(cursors *cs) {
    return cs->a[cs->current].col;
}
*/
#ifdef cursorsTest

// TODO: test.

// Represent left-to-right cursor as [..} and right-to-left as {..]. In each
// test, the cursors start non-overlapping, and merging is done so that they
// should end up non-overlapping.

int main() {
    printf("Cursors module OK\n");
    return 0;
}

#endif
