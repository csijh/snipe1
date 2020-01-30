#include "event.h"
#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

event getEvent();

struct display {
    ALLEGRO_DISPLAY *window;
    ALLEGRO_EVENT_QUEUE *queue;
    ALLEGRO_FONT *font;
    ALLEGRO_COLOR bg, fg;
    ALLEGRO_TIMER* timer;
    ALLEGRO_EVENT e;
    int unichar, x, y;
    bool dragging;
};
typedef struct display display;

char *lines[] = {
"ISC License π́  ",
"",
"Copyright (c) 2017-2018, Ian Holyer",
"",
"Permission to use, copy, modify, and/or distribute this software for any",
"purpose with or without fee is hereby granted, provided that the above",
"copyright notice and this permission notice appear in all copies.",
"",
"THE SOFTWARE IS PROVIDED 'AS IS' AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH",
"REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY",
"AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,",
"INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM",
"LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE",
"OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR",
"PERFORMANCE OF THIS SOFTWARE."
};
int nlines = 15;

// Crash the program if there is any failure.
void fail(char *s) {
    fprintf(stderr, "%s\n", s);
    exit(1);
}

// Draw a screenful of text.
void drawPage(ALLEGRO_FONT *font, ALLEGRO_COLOR bg, ALLEGRO_COLOR fg) {
    al_clear_to_color(bg);
    float x = 4, y = 0;
    int n = al_get_font_line_height(font);
    for (int i = 0; i < nlines; i++) {
        al_draw_text(font, fg, x, y, 0, lines[i]);
        y += n;
    }
    al_flip_display();
}

int main(int argc, char **argv){
    display *d = malloc(sizeof(display));
    d->dragging = false;
    bool ok;

    ok = al_init();
    if (! ok) fail("Failed to initialize Allegro.");
    ok = al_install_keyboard();
    if (! ok) fail("Failed to initialize keyboard.");
    ok = al_install_mouse();
    if (! ok) fail("Failed to initialize mouse.");
    al_init_font_addon();
    al_init_ttf_addon();

    al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_RESIZABLE);
    d->window = al_create_display(890, 530);
    if (d->window == NULL) fail("Failed to create display.");
    al_set_system_mouse_cursor(d->window, ALLEGRO_SYSTEM_MOUSE_CURSOR_EDIT);
    d->queue = al_create_event_queue();
    if (d->queue == NULL) fail("Failed to create queue.");
    d->font = al_load_ttf_font("../fonts/DejaVuSansMono.ttf", 18, 0);
    if (d->font == NULL) fail("Failed to load 'DejaVuSansMono.ttf'.");
    d->bg = al_map_rgb(0, 0x2b, 0x36);
    d->fg = al_map_rgb(0x83, 0x94, 0x96);
    d->timer = al_create_timer(10.0);
    if (d->timer == NULL) fail("Failed to create timer.");
    al_register_event_source(d->queue, al_get_display_event_source(d->window));
    al_register_event_source(d->queue, al_get_keyboard_event_source());
    al_register_event_source(d->queue, al_get_mouse_event_source());
    al_register_event_source(d->queue, al_get_timer_event_source(d->timer));

    drawPage(d->font, d->bg, d->fg);
    al_start_timer(d->timer);
    ok = true;
    while (ok) {
        event e = getEvent(d);
        printf("%s", findEventName(e));
        if (e == TEXT) printf(" %c %d", d->unichar, d->unichar);
        if (e == CLICK || e == DRAG || e == UNCLICK) {
            printf(" %d %d", d->x, d->y);
        }
        printf("\n");
        if (e == QUIT) ok = false;
    }
    al_destroy_display(d->window);
    al_destroy_event_queue(d->queue);
    al_destroy_font(d->font);
    al_destroy_timer(d->timer);
    al_shutdown_ttf_addon();
    al_shutdown_font_addon();
    al_uninstall_mouse();
    al_uninstall_keyboard();
    al_uninstall_system();
    free(d);
    return 0;
}

// Convert a non-text keycode into a logical event, or return -1.
event nonText(int keycode) {
    switch (keycode) {
        case ALLEGRO_KEY_ESCAPE: return ESCAPE;
        case ALLEGRO_KEY_ENTER: return ENTER;
        case ALLEGRO_KEY_PAD_ENTER: return ENTER;
        case ALLEGRO_KEY_TAB: return TAB;
        case ALLEGRO_KEY_BACKSPACE: return BACKSPACE;
        case ALLEGRO_KEY_INSERT: return INSERT;
        case ALLEGRO_KEY_DELETE: return DELETE;
        case ALLEGRO_KEY_LEFT: return LEFT;
        case ALLEGRO_KEY_RIGHT: return RIGHT;
        case ALLEGRO_KEY_UP: return UP;
        case ALLEGRO_KEY_DOWN: return DOWN;
        case ALLEGRO_KEY_PGUP: return PAGE_UP;
        case ALLEGRO_KEY_PGDN: return PAGE_DOWN;
        case ALLEGRO_KEY_HOME: return HOME;
        case ALLEGRO_KEY_END: return END;
        case ALLEGRO_KEY_F1: return F1;
        case ALLEGRO_KEY_F2: return F2;
        case ALLEGRO_KEY_F3: return F3;
        case ALLEGRO_KEY_F4: return F4;
        case ALLEGRO_KEY_F5: return F5;
        case ALLEGRO_KEY_F6: return F6;
        case ALLEGRO_KEY_F7: return F7;
        case ALLEGRO_KEY_F8: return F8;
        case ALLEGRO_KEY_F9: return F9;
        case ALLEGRO_KEY_F10: return F10;
        case ALLEGRO_KEY_F11: return F11;
        case ALLEGRO_KEY_F12: return F12;
        default: return -1;
    }
}

// Convert control plus a text keycode into a logical event, or return -1. No
// keyboard layout info is available, so '+' is assumed to be on the '=' key.
event controlText(int keycode) {
    switch (keycode) {
        case ALLEGRO_KEY_A: return C_A;
        case ALLEGRO_KEY_B: return C_B;
        case ALLEGRO_KEY_C: return C_C;
        case ALLEGRO_KEY_D: return C_D;
        case ALLEGRO_KEY_E: return C_E;
        case ALLEGRO_KEY_F: return C_F;
        case ALLEGRO_KEY_G: return C_G;
        case ALLEGRO_KEY_H: return C_H;
        case ALLEGRO_KEY_I: return C_I;
        case ALLEGRO_KEY_J: return C_J;
        case ALLEGRO_KEY_K: return C_K;
        case ALLEGRO_KEY_L: return C_L;
        case ALLEGRO_KEY_M: return C_M;
        case ALLEGRO_KEY_N: return C_N;
        case ALLEGRO_KEY_O: return C_O;
        case ALLEGRO_KEY_P: return C_P;
        case ALLEGRO_KEY_Q: return C_Q;
        case ALLEGRO_KEY_R: return C_R;
        case ALLEGRO_KEY_S: return C_S;
        case ALLEGRO_KEY_T: return C_T;
        case ALLEGRO_KEY_U: return C_U;
        case ALLEGRO_KEY_V: return C_V;
        case ALLEGRO_KEY_W: return C_W;
        case ALLEGRO_KEY_X: return C_X;
        case ALLEGRO_KEY_Y: return C_Y;
        case ALLEGRO_KEY_Z: return C_Z;
        case ALLEGRO_KEY_0: case ALLEGRO_KEY_PAD_0: return C_0;
        case ALLEGRO_KEY_1: case ALLEGRO_KEY_PAD_1: return C_1;
        case ALLEGRO_KEY_2: case ALLEGRO_KEY_PAD_2: return C_2;
        case ALLEGRO_KEY_3: case ALLEGRO_KEY_PAD_3: return C_3;
        case ALLEGRO_KEY_4: case ALLEGRO_KEY_PAD_4: return C_4;
        case ALLEGRO_KEY_5: case ALLEGRO_KEY_PAD_5: return C_5;
        case ALLEGRO_KEY_6: case ALLEGRO_KEY_PAD_6: return C_6;
        case ALLEGRO_KEY_7: case ALLEGRO_KEY_PAD_7: return C_7;
        case ALLEGRO_KEY_8: case ALLEGRO_KEY_PAD_8: return C_8;
        case ALLEGRO_KEY_9: case ALLEGRO_KEY_PAD_9: return C_9;
        case ALLEGRO_KEY_EQUALS: case ALLEGRO_KEY_PAD_PLUS: return C_PLUS;
        case ALLEGRO_KEY_MINUS: case ALLEGRO_KEY_PAD_MINUS: return C_MINUS;
        default: return -1;
    }
}

// Get an event, converting physical events into logical events. Some physical
// events are handled internally and are not reported. CHAR events are used for
// text (so shift is interpreted according to keyboard layout and Unicode input
// is supported) and not for controls (because some platforms don't generate
// CHAR events for controls).
event getEvent(display *d) {
    int key, mods;
    bool shift, ctrl, cmd;
    ALLEGRO_KEYBOARD_STATE keys;
    while (true) {
        al_wait_for_event(d->queue, &d->e);
        switch(d->e.type) {
            case ALLEGRO_EVENT_DISPLAY_CLOSE: return QUIT;
            case ALLEGRO_EVENT_DISPLAY_SWITCH_OUT: return BLUR;
            case ALLEGRO_EVENT_DISPLAY_SWITCH_IN: return FOCUS;
            case ALLEGRO_EVENT_DISPLAY_RESIZE:
                al_acknowledge_resize(d->window);
                drawPage(d->font, d->bg, d->fg);
                return RESIZE;
            case ALLEGRO_EVENT_KEY_DOWN:
                key = d->e.keyboard.keycode;
                al_get_keyboard_state(&keys);
                shift =
                    al_key_down(&keys, ALLEGRO_KEY_LSHIFT) ||
                    al_key_down(&keys, ALLEGRO_KEY_RSHIFT);
                ctrl =
                    al_key_down(&keys, ALLEGRO_KEY_LCTRL) ||
                    al_key_down(&keys, ALLEGRO_KEY_RCTRL);
                cmd = al_key_down(&keys, ALLEGRO_KEY_COMMAND);
                event e = nonText(key);
                if (e < 0 && (ctrl|cmd)) {
                    e = controlText(key);
                    if (e >= 0) return e;
                }
                if (e < 0) break;
                if (shift && (ctrl|cmd)) e = SC_ + e;
                else if (shift) e = S_ + e;
                else if (ctrl|cmd) e = C_ + e;
                return e;
            case ALLEGRO_EVENT_KEY_UP:
                break;
            case ALLEGRO_EVENT_KEY_CHAR:
                d->unichar = d->e.keyboard.unichar;
                if (d->unichar < ' ' || d->unichar == 127) break;
                mods = d->e.keyboard.modifiers;
                ctrl = (mods & ALLEGRO_KEYMOD_CTRL) != 0;
                cmd = (mods & ALLEGRO_KEYMOD_COMMAND) != 0;
                if (ctrl | cmd) break;
                return TEXT;
            case ALLEGRO_EVENT_MOUSE_AXES:
                if (! d->dragging) break;
                d->x = d->e.mouse.x;
                d->y = d->e.mouse.y;
                return DRAG;
            case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
                d->dragging = true;
                return CLICK;
            case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
                d->dragging = false;
                return UNCLICK;
            case ALLEGRO_EVENT_TIMER:
                printf("tick\n");
                break;
            default:
                break;
        }
    }
}

/*
case ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY:
    printf("enter\n");
    break;
case ALLEGRO_EVENT_MOUSE_LEAVE_DISPLAY:
    printf("leave\n");
    break;
*/
