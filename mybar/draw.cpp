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

#include "draw.h"

#include <string>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <malloc.h>
#include <string.h>

#define ALIGN_LEFT 0
#define ALIGN_CENTER 1
#define ALIGN_RIGHT 2

#define DEFAULT_BG "#222222"
#define DEFAULT_FG "#999999"
#define DEFAULT_FONT "-*-terminus-medium-*-*-*-12-*-*-*-*-*-*-*"
#define X 0
#define Y 0
#define W 1280
#define H 20

typedef struct
{
    int position;
    int width;
    std::string text;
    uint32_t fg;
    uint32_t bg;
} text_format;

std::vector<text_format> text_format_list;

xcb_connection_t * connection;
xcb_window_t window;
xcb_font_t font;
xcb_colormap_t colormap;

bool running = true;

uint32_t fg;
uint32_t bg;
int cursor = 0;
int text_alignment = ALIGN_LEFT;

uint32_t default_fg;
uint32_t default_bg;

void add_text_format(const std::string & string);
void draw_format_list();
uint32_t alloc_color(const std::string & color_string);

xcb_char2b_t * build_chars(const std::string & string)
{
    xcb_char2b_t * chars = (xcb_char2b_t *)malloc(string.size() * sizeof(xcb_char2b_t));
    if (!chars)
    {
        printf("Couldn't allocate characters\n");
        return NULL;
    }

    int i;
    for (i = 0; i < string.size(); i++)
    {
        chars[i].byte1 = 0;
        chars[i].byte2 = string[i];
    }

    return chars;
}

void draw_init()
{
    xcb_screen_t * screen;

    connection = xcb_connect(NULL, NULL);
    screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;

    colormap = screen->default_colormap;

    font = xcb_generate_id(connection);
    xcb_open_font(
        connection,
        font,
        strlen(DEFAULT_FONT),
        DEFAULT_FONT
    );

    default_bg = alloc_color(DEFAULT_BG);
    default_fg = alloc_color(DEFAULT_FG);

    printf("font is %i\n", font);

    window = xcb_generate_id(connection);
    uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK;
    uint32_t values[3];
    values[0] = default_bg;
    values[1] = 1;
    values[2] = XCB_EVENT_MASK_EXPOSURE;
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

    xcb_map_window(connection, window);

    xcb_flush(connection);
}

void draw_loop()
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

void draw_cleanup()
{
    xcb_close_font(connection, font);
    xcb_disconnect(connection);
}

void draw(const std::string & input)
{
    size_t offset = 0;
    size_t marker = 0;

    cursor = 0;
    bg = default_bg;
    fg = default_fg;

    while((marker = input.find('^', marker)) != std::string::npos)
    {
        if (input.size() > (marker + 1))
        {
            if (input[marker + 1] == '^')
            {
                marker = marker + 2;
                continue;
            }
        }
        else
        {
            std::cout << "Error in formatting string: trailing ^\n";
            text_format_list.clear();
            return;
        }

        if (marker > offset)
        {
            std::string string = input.substr(offset, marker - offset);
            size_t carrot_position = 0;
            while ((carrot_position = string.find("^^", carrot_position)) != std::string::npos)
            {
                std::cout << "replacing at " << carrot_position << "\n";
                string.replace(carrot_position, 2, "^");
            }
            add_text_format(string);
            std::cout << "string: " << string << "\n";
        }

        size_t arg_begin, arg_end;
        arg_begin = input.find('(', marker);
        if (arg_begin == std::string::npos)
        {
            std::cout << "Error in formatting string: couldn't find opening (\n";
            text_format_list.clear();
            return;
        }
        arg_end = input.find(')', arg_begin);
        if (arg_end == std::string::npos)
        {
            std::cout << "Error in formatting string: couldn't find closing )\n";
            text_format_list.clear();
            return;
        }

        std::string command = input.substr(marker + 1, arg_begin - marker - 1);
        std::string arg = input.substr(arg_begin + 1, arg_end - arg_begin - 1);

        std::cout << "command: " << command << ", arg: " << arg << "\n";

        if (command == "p")
        {
            int space = atoi(arg.c_str());
            cursor += space;
        }
        else if (command == "bg")
        {
            bg = alloc_color(arg);
        }
        else if (command == "fg")
        {
            fg = alloc_color(arg);
        }
        else if (command == "i")
        {
            std::cout << "Images not supported yet\n";
        }
        else
        {
            std::cout << "command '" << command << "' not recognized\n";
        }

        offset = arg_end + 1;
        marker = offset;
    }
    if (input.size() > offset)
    {
        add_text_format(input.substr(offset));
        std::cout << "string: " << input.substr(offset) << "\n";
    }
    
    xcb_get_geometry_reply_t * geometry = xcb_get_geometry_reply(
        connection,
        xcb_get_geometry(
            connection,
            window
        ),
        NULL
    );
    xcb_clear_area(
        connection,
        1,
        window,
        geometry->x, geometry->y,
        geometry->width, geometry->height
    );
    draw_format_list();
    xcb_flush(connection);
    text_format_list.clear();
    cursor = 0;
}

void add_text_format(const std::string & string)
{
    text_format format;
    format.text = string;
    format.fg = fg;
    format.bg = bg;
    format.position = cursor;

    xcb_query_text_extents_cookie_t text_extents_cookie = xcb_query_text_extents(
        connection,
        font,
        string.size(),
        build_chars(string)
    );
    xcb_query_text_extents_reply_t * text_extents = xcb_query_text_extents_reply(
        connection,
        text_extents_cookie,
        NULL
    );

    format.width = text_extents->overall_width;

    text_format_list.push_back(format);

    cursor += format.width;
}

uint32_t alloc_color(const std::string & color_string)
{
    if (color_string[0] == '#')
    {
        int color = std::strtol(color_string.substr(1).c_str(), NULL, 16);
        uint16_t red = 0;
        uint16_t green = 0;
        uint16_t blue = 0;
        red = (color & 0xFF0000) >> 8;
        green = (color & 0x00FF00);
        blue = (color & 0x0000FF) << 8;
        xcb_alloc_color_reply_t * reply = xcb_alloc_color_reply(
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
        return reply->pixel;
    }
    else
    {
        xcb_alloc_named_color_cookie_t color_cookie = xcb_alloc_named_color(
            connection,
            colormap,
            color_string.size(),
            color_string.c_str()
        );
        xcb_alloc_named_color_reply_t * color = xcb_alloc_named_color_reply(
            connection,
            color_cookie,
            NULL
        );
        return color->pixel;
    }

}

void draw_format_list()
{
    std::cout << "draw format list, size: " << text_format_list.size() << "\n";
    xcb_gcontext_t gc;
    if (text_alignment == ALIGN_LEFT)
    {
        for (std::vector<text_format>::const_iterator format = text_format_list.begin(); format != text_format_list.end(); ++format)
        {
            std::cout << "drawing text: " << (*format).text << "\n";
            gc = xcb_generate_id(connection);
            uint32_t mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND | XCB_GC_FONT;
            uint32_t values[3];
            values[0] = (*format).fg;
            values[1] = (*format).bg;
            values[2] = font;

            xcb_create_gc(
                connection,
                gc,
                window,
                mask,
                values
            );

            xcb_image_text_8(
                connection,
                (*format).text.size(),
                window,
                gc,
                (*format).position, 10,
                (*format).text.c_str()
            );
        }
    }
}

// void draw_text(const char * string, size_t length)
// {
//     xcb_image_text_8(
//         connection,
//         length,
//         window,
//         gc,
//         cursor, 10,
//         string
//     );
// }

