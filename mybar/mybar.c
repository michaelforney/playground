/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2009, Michael Forney <michael@obberon.com>
 */

#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <xcb/xcb.h>
#include <pthread.h>

#include "input.h"

#define X 0
#define Y 0
#define W 1280
#define H 20

#define BG "#222222"
#define FG "#999999"

xcb_connection_t * connection;
xcb_window_t window;
xcb_colormap_t colormap;
xcb_font_t font;

pthread_t input_thread;

//uint32_t mask;
int running = 1;

xcb_alloc_color_reply_t * alloc_color(const char * color_string);
void event_loop();
void cleanup();
void draw(int x, int y, int w, int h);

int main(int argc, char * argv[])
{
    xcb_screen_t *      screen;

    connection = xcb_connect(NULL, NULL);
    screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;

    colormap = screen->default_colormap;

    font = xcb_generate_id(connection);
    {
        const char * font_name = "-*-terminus-medium-*-*-*-12-*-*-*-*-*-*-*";
        xcb_open_font(
            connection,
            font,
            strlen(font_name),
            font_name
        );
    }

    window = xcb_generate_id(connection);
    {
        uint32_t mask = XCB_CW_BACK_PIXEL
            | XCB_CW_OVERRIDE_REDIRECT
            | XCB_CW_EVENT_MASK
        ;
        uint32_t values[3];
        values[0] = alloc_color(BG)->pixel;
        values[1] = 1;
        values[2] = XCB_EVENT_MASK_EXPOSURE
                    //| XCB_EVENT_MASK_BUTTON_PRESS
                    //| XCB_EVENT_MASK_BUTTON_RELEASE
                    //| XCB_EVENT_MASK_POINTER_MOTION
                    //| XCB_EVENT_MASK_ENTER_WINDOW
                    //| XCB_EVENT_MASK_LEAVE_WINDOW
                    //| XCB_EVENT_MASK_KEY_PRESS
                    //| XCB_EVENT_MASK_KEY_RELEASE
        ;
        xcb_create_window(
            connection,
            XCB_COPY_FROM_PARENT,
            window,
            screen->root,
            X, Y,
            W, H,
            0,
            XCB_WINDOW_CLASS_INPUT_OUTPUT,
            screen->root_visual,
            mask, values
        );
    }

    xcb_map_window(connection, window);

    xcb_flush(connection);

    input_thread = pthread_create(&input_thread, NULL, input_loop, NULL);
    event_loop();
    cleanup();

    return 0;
}

xcb_alloc_color_reply_t * alloc_color(const char * color_string)
{
    uint16_t red = 0;
    uint16_t green = 0;
    uint16_t blue = 0;
    if (color_string[0] == '#')
    {
        int color = strtol(color_string + 1, NULL, 16);
        red = (color & 0xFF0000) >> 8;
        green = (color & 0x00FF00);
        blue = (color & 0x0000FF) << 8;
    }
    return xcb_alloc_color_reply(
        connection,
        xcb_alloc_color(
            connection,
            colormap,
            red,
            green,
            blue
        ),
        NULL
    );
}

void event_loop()
{
    xcb_generic_event_t * event;
    while ((event = xcb_wait_for_event(connection)) && running)
    {
        switch (event->response_type & ~0x80)
        {
            case XCB_EXPOSE:
            {
                printf("expose\n");
                xcb_expose_event_t * expose_event;
                draw(
                    expose_event->x,
                    expose_event->y,
                    expose_event->width,
                    expose_event->height
                );
                break;
            }
            default:
            {
                break;
            }
        }
        free(event);
    }
}

void draw(int x, int y, int w, int h)
{
    xcb_gcontext_t gc;
    uint32_t mask;
    uint32_t values[3];
    
    gc = xcb_generate_id(connection);
    mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND | XCB_GC_FONT;
    values[0] = alloc_color(FG)->pixel;
    values[1] = alloc_color(BG)->pixel;
    values[2] = font;
    xcb_create_gc(
        connection,
        gc,
        window,
        mask,
        values
    );

    const char * string = "Hello, XCB!";
    xcb_image_text_8(
        connection,
        strlen(string),
        window,
        gc,
        0, 12,
        string
    );
    xcb_flush(connection);
}

void cleanup()
{
    xcb_close_font(connection, font);
    xcb_disconnect(connection);
}

