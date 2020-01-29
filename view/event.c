// Events. Free and open source. See LICENSE.
#include "event.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

// Event names.
static char *eventNames[] = {
    [ESCAPE]="ESCAPE", [S_ESCAPE]="S_ESCAPE", [C_ESCAPE]="C_ESCAPE",
    [SC_ESCAPE]="SC_ESCAPE", [ENTER]="ENTER", [S_ENTER]="S_ENTER",
    [C_ENTER]="C_ENTER", [SC_ENTER]="SC_ENTER", [TAB]="TAB", [S_TAB]="S_TAB",
    [C_TAB]="C_TAB", [SC_TAB]="SC_TAB", [BACKSPACE]="BACKSPACE",
    [S_BACKSPACE]="S_BACKSPACE", [C_BACKSPACE]="C_BACKSPACE",
    [SC_BACKSPACE]="SC_BACKSPACE", [INSERT]="INSERT", [S_INSERT]="S_INSERT",
    [C_INSERT]="C_INSERT", [SC_INSERT]="SC_INSERT", [DELETE]="DELETE",
    [S_DELETE]="S_DELETE", [C_DELETE]="C_DELETE", [SC_DELETE]="SC_DELETE",
    [LEFT]="LEFT", [S_LEFT]="S_LEFT", [C_LEFT]="C_LEFT", [SC_LEFT]="SC_LEFT",
    [RIGHT]="RIGHT", [S_RIGHT]="S_RIGHT", [C_RIGHT]="C_RIGHT",
    [SC_RIGHT]="SC_RIGHT", [UP]="UP", [S_UP]="S_UP", [C_UP]="C_UP",
    [SC_UP]="SC_UP", [DOWN]="DOWN", [S_DOWN]="S_DOWN", [C_DOWN]="C_DOWN",
    [SC_DOWN]="SC_DOWN", [PAGE_UP]="PAGE_UP", [S_PAGE_UP]="S_PAGE_UP",
    [C_PAGE_UP]="C_PAGE_UP", [SC_PAGE_UP]="SC_PAGE_UP", [PAGE_DOWN]="PAGE_DOWN",
    [S_PAGE_DOWN]="S_PAGE_DOWN", [C_PAGE_DOWN]="C_PAGE_DOWN",
    [SC_PAGE_DOWN]="SC_PAGE_DOWN", [HOME]="HOME", [S_HOME]="S_HOME",
    [C_HOME]="C_HOME", [SC_HOME]="SC_HOME", [END]="END", [S_END]="S_END",
    [C_END]="C_END", [SC_END]="SC_END", [F1]="F1", [S_F1]="S_F1", [C_F1]="C_F1",
    [SC_F1]="SC_F1", [F2]="F2", [S_F2]="S_F2", [C_F2]="C_F2", [SC_F2]="SC_F2",
    [F3]="F3", [S_F3]="S_F3", [C_F3]="C_F3", [SC_F3]="SC_F3", [F4]="F4",
    [S_F4]="S_F4", [C_F4]="C_F4", [SC_F4]="SC_F4", [F5]="F5", [S_F5]="S_F5",
    [C_F5]="C_F5", [SC_F5]="SC_F5", [F6]="F6", [S_F6]="S_F6", [C_F6]="C_F6",
    [SC_F6]="SC_F6", [F7]="F7", [S_F7]="S_F7", [C_F7]="C_F7", [SC_F7]="SC_F7",
    [F8]="F8", [S_F8]="S_F8", [C_F8]="C_F8", [SC_F8]="SC_F8", [F9]="F9",
    [S_F9]="S_F9", [C_F9]="C_F9", [SC_F9]="SC_F9", [F10]="F10", [S_F10]="S_F10",
    [C_F10]="C_F10", [SC_F10]="SC_F10", [F11]="F11", [S_F11]="S_F11",
    [C_F11]="C_F11", [SC_F11]="SC_F11", [F12]="F12", [S_F12]="S_F12",
    [C_F12]="C_F12", [SC_F12]="SC_F12", [CLICK]="CLICK", [S_CLICK]="S_CLICK",
    [C_CLICK]="C_CLICK", [SC_CLICK]="SC_CLICK", [DRAG]="DRAG",
    [S_DRAG]="S_DRAG", [C_DRAG]="C_DRAG", [SC_DRAG]="SC_DRAG",
    [UNCLICK]="UNCLICK", [S_UNCLICK]="S_UNCLICK", [C_UNCLICK]="C_UNCLICK",
    [SC_UNCLICK]="SC_UNCLICK", [SCROLL]="SCROLL", [S_SCROLL]="S_SCROLL",
    [C_SCROLL]="C_SCROLL", [SC_SCROLL]="SC_SCROLL", [TEXT]="TEXT",
    [C_TEXT]="C_TEXT", [PASTE]="PASTE", [RESIZE]="RESIZE", [FOCUS]="FOCUS",
    [BLUR]="BLUR", [FRAME]="FRAME", [LOAD]="LOAD", [BLINK]="BLINK",
    [SAVE]="SAVE", [QUIT]="QUIT", [IGNORE]="IGNORE"
};
static int COUNT = sizeof(eventNames) / sizeof(char *);

const char *findEventName(event e) {
    return eventNames[e];
}

event findEvent(char *name) {
    for (event e = 0; e < COUNT; e++) {
        if (strcmp(eventNames[e], name) == 0) return e;
    }
    printf("Unknown event name %s\n", name);
    exit(1);
}

void printEvent(event e, int x, int y, char const *t, char *end) {
    printf("%s", findEventName(e));
    if (e == TEXT) printf(" %s", t);
    else if (e == CLICK || e == DRAG || e == UNCLICK || e == SCROLL) {
        printf(" x=%d y=%d", x, y);
    }
    printf("%s", end);
}

#ifdef eventTest

static bool startsWith(const char *s, const char *prefix) {
    return strncmp(s, prefix, strlen(prefix)) == 0;
}

// Check that e+1, e+2, e+3 are the same as e with prefixes S_ C_ SC_.
static bool checkPrefixes(event e) {
    if (eventNames[e] == NULL) printf("null %d\n", e);
    if (eventNames[e+1] == NULL) printf("null %d\n", e+1);
    if (eventNames[e+2] == NULL) printf("null %d\n", e+2);
    if (eventNames[e+3] == NULL) printf("null %d\n", e+3);
    if (! startsWith(eventNames[e+1], "S_")) return false;
    if (! startsWith(eventNames[e+2], "C_")) return false;
    if (! startsWith(eventNames[e+3], "SC_")) return false;
    if (strcmp(eventNames[e+1]+2, eventNames[e]) != 0) return false;
    if (strcmp(eventNames[e+2]+2, eventNames[e]) != 0) return false;
    if (strcmp(eventNames[e+3]+3, eventNames[e]) != 0) return false;
    return true;
}

// Check that events up to TEXT come in groups of four.
static void checkGroups() {
    for (int e = 0; e < TEXT; e = e + 4) {
        assert(checkPrefixes(e));
    }
}

int main(int n, char const *args[]) {
    setbuf(stdout, NULL);
    checkGroups();
    printf("Event module OK\n");
    return 0;
}

#endif
