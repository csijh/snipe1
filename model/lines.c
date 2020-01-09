// Lines of text. Free and open source. See LICENSE.
#include "lines.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

// Store an array holding the position in the text just after each newline. The
// array is organised as a gap buffer from 0 to end, with the gap between lo and
// hi. The total number of bytes in the text is tracked in max, and entries
// after the gap are stored relative to max. This allows the lines to be tracked
// incrementally, using only the insertions and deletions.
struct lines {
    int *a;
    int lo, hi, end, max;
};

lines *newLines() {
    lines *ls = malloc(sizeof(lines));
    int *a = malloc(6 * sizeof(int));
    *ls = (lines) { .lo=0, .hi=6, .end=6, .max=0, .a=a };
    return ls;
}

void freeLines(lines *ls) {
    free(ls->a);
    free(ls);
}

// Move the gap to the given text position.
static void moveGap(lines *ls, int at) {
    while (ls->lo > 0 && ls->a[ls->lo - 1] > at) {
        ls->lo--;
        ls->hi--;
        ls->a[ls->hi] = ls->max - ls->a[ls->lo];
    }
    while (ls->hi < ls->end && ls->max - ls->a[ls->hi] <= at) {
        ls->a[ls->lo] = ls->max - ls->a[ls->hi];
        ls->hi++;
        ls->lo++;
    }
}

static void resize(lines *ls) {
    int size = ls->end;
    size = size * 3 / 2;
    ls->a = realloc(ls->a, size * sizeof(int));
    int hilen = ls->end - ls->hi;
    memmove(&ls->a[size - hilen], &ls->a[ls->hi], hilen * sizeof(int));
    ls->hi = size - hilen;
    ls->end = size;
}

void insertLines(lines *ls, int at, int n, char const s[n]) {
    ls->max = ls->max + n;
    moveGap(ls, at);
    for (int i = 0; i < n; i++) if (s[i] == '\n') {
        if (ls->lo >= ls->hi) resize(ls);
        ls->a[ls->lo++] = at + i + 1;
    }
}

void deleteLines(lines *ls, int at, int n, char const s[n]) {
    ls->max = ls->max - n;
    moveGap(ls, at);
    while (ls->lo > 0 && ls->a[ls->lo - 1] > at - n) {
        ls->lo--;
    }
}

int countLines(lines *ls) {
    return ls->end - (ls->hi - ls->lo);
}

int startLine(lines *ls, int row) {
    int n = countLines(ls);
    if (n == 0) return 0;
    if (row < 0) row = 0; else if (row >= n) row = n - 1;
    if (row == 0) return 0;
    else if (row <= ls->lo) return ls->a[row - 1];
    else return ls->max - ls->a[row + (ls->hi - ls->lo) - 1];
}

int endLine(lines *ls, int row) {
    int n = countLines(ls);
    if (n == 0) return 0;
    if (row < 0) row = 0; else if (row >= n) row = n - 1;
    if (row < ls->lo) return ls->a[row];
    else return ls->max - ls->a[row + (ls->hi - ls->lo)];
}

int lengthLine(lines *ls, int row) {
    return endLine(ls, row) - startLine(ls, row);
}

// Find the row number for a position by binary search.
int findRow(lines *ls, int at) {
    if (at < 0) at = 0; else if (at > ls->max) at = ls->max;
    int start = 0, end = countLines(ls);
    while (end > start) {
        int mid = start + (end - start) / 2;
        int s = endLine(ls, mid);
        if (at < s) end = mid;
        else start = mid + 1;
    }
    return start;
}

#ifdef linesTest

// Check the lines structure against an array of positions.
static bool check(lines *ls, int n, int a[n]) {
    if (countLines(ls) != n) return false;
    for (int i=0; i<n; i++) {
        if (endLine(ls, i) != a[i]) return false;
    }
    return true;
}

// Test insertions
static void testInsert(lines *ls) {
    insertLines(ls, 0, 3, "ab\n");
    assert(check(ls, 1, (int[]){3}));
    insertLines(ls, 3, 4, "cde\n");
    assert(check(ls, 2, (int[]){3, 7}));
    insertLines(ls, 7, 5, "fghi\n");
    assert(check(ls, 3, (int[]){3, 7, 12}));
}

// Test findRow.
static void testFind(lines *ls) {
    assert(findRow(ls, 0) == 0);
    assert(findRow(ls, 2) == 0);
    assert(findRow(ls, 3) == 1);
    assert(findRow(ls, 6) == 1);
    assert(findRow(ls, 7) == 2);
    assert(findRow(ls, 11) == 2);
    assert(findRow(ls, 12) == 3);
}

// Test startLine, endLine, lengthLine.
static void testLines(lines *ls) {
    assert(startLine(ls, 0) == 0);
    assert(endLine(ls, 0) == 3);
    assert(lengthLine(ls, 0) == 3);
    assert(startLine(ls, 1) == 3);
    assert(endLine(ls, 1) == 7);
    assert(lengthLine(ls, 1) == 4);
    assert(startLine(ls, 2) == 7);
    assert(endLine(ls, 2) == 12);
    assert(lengthLine(ls, 2) == 5);
}

// Test deletions
static void testDelete(lines *ls) {
    deleteLines(ls, 12, 5, "fghi\n");
    assert(check(ls, 2, (int[]){3, 7}));
    deleteLines(ls, 7, 4, "cde\n");
    assert(check(ls, 1, (int[]){3}));
    deleteLines(ls, 3, 3, "ab\n");
    check(ls, 0, NULL);
}

int main() {
    setbuf(stdout, NULL);
    lines *ls = newLines();
    check(ls, 0, NULL);
    assert(findRow(ls, 0) == 0);
    testInsert(ls);
    testFind(ls);
    testLines(ls);
    testDelete(ls);
    freeLines(ls);
    printf("Lines module OK\n");
    return 0;
}

#endif
