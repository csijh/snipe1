#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

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
    ALLEGRO_DISPLAY *display;
    ALLEGRO_EVENT_QUEUE *queue;
    ALLEGRO_FONT *font;
    ALLEGRO_COLOR bg, fg;
    ALLEGRO_TIMER* timer;
    bool ok;

    ok = al_init();
    if (! ok) fail("Failed to initialize Allegro.");
    al_init_font_addon();
    al_init_ttf_addon();

    al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_RESIZABLE);
    display = al_create_display(890, 530);
    if (display == NULL) fail("Failed to create display.");
    queue = al_create_event_queue();
    if (queue == NULL) fail("Failed to create queue.");
    al_register_event_source(queue, al_get_display_event_source(display));
    font = al_load_ttf_font("DejaVuSansMono.ttf", 18, 0);
    if (font == NULL) fail("Failed to load 'DejaVuSansMono.ttf'.");
    bg = al_map_rgb(0, 0x2b, 0x36);
    fg = al_map_rgb(0x83, 0x94, 0x96);
    ok = al_install_keyboard();
    al_register_event_source(queue, al_get_keyboard_event_source());
    if (! ok) fail("Failed to set up keyboard.");
    timer = al_create_timer(5.0);
    if (timer == NULL) fail("Failed to create timer.");
    al_register_event_source(queue, al_get_timer_event_source(timer));

    drawPage(font, bg, fg);
    al_start_timer(timer);
    ok = true;
    ALLEGRO_EVENT e;
    while (ok) {
        al_wait_for_event(queue, &e);
        switch(e.type) {
            case ALLEGRO_EVENT_DISPLAY_CLOSE: ok = false; break;
            case ALLEGRO_EVENT_DISPLAY_SWITCH_OUT:
                printf("blur %d\n", e.type);
                break;
            case ALLEGRO_EVENT_DISPLAY_SWITCH_IN:
                printf("focus %d\n", e.type);
                break;
            case ALLEGRO_EVENT_KEY_DOWN:
                printf("key down %d\n", e.type);
                break;
            case ALLEGRO_EVENT_KEY_UP:
                printf("key up %d\n", e.type);
                break;
            case ALLEGRO_EVENT_KEY_CHAR:
                printf("key char %d\n", e.type);
                break;
            case ALLEGRO_EVENT_TIMER:
                printf("tick\n");
                break;
            case ALLEGRO_EVENT_DISPLAY_RESIZE:
                al_acknowledge_resize(display);
                drawPage(font, bg, fg);
                break;
            default:
            printf("e %d\n", e.type);
        }
    }

    al_destroy_display(display);
    return 0;
}

//   ALLEGRO_USTR_INFO ui;
//   ui.mlen = 42;
//   ALLEGRO_USTR *us = &ui;
//   printf("%d\n", us->mlen);
/*
case ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY:
    printf("enter\n");
    break;
case ALLEGRO_EVENT_MOUSE_LEAVE_DISPLAY:
    printf("leave\n");
    break;
case ALLEGRO_EVENT_DISPLAY_EXPOSE:
case ALLEGRO_EVENT_DISPLAY_LOST:
    printf("lost %d\n", e.type);
    break;
case ALLEGRO_EVENT_DISPLAY_FOUND:
    printf("found %d\n", e.type);
    break;
case ALLEGRO_EVENT_DISPLAY_ORIENTATION:
    printf("or %d\n", e.type);
    break;
case ALLEGRO_EVENT_DISPLAY_HALT_DRAWING:
    printf("hlt %d\n", e.type);
    break;
case ALLEGRO_EVENT_DISPLAY_RESUME_DRAWING:
    printf("res %d\n", e.type);
    break;
case ALLEGRO_EVENT_DISPLAY_CONNECTED:
    printf("con %d\n", e.type);
    break;
case ALLEGRO_EVENT_DISPLAY_DISCONNECTED:
    printf("disc %d\n", e.type);
    break;
*/
