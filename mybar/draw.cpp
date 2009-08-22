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
#include <fstream>
#include <cstdlib>
#include <assert.h>
#include <malloc.h>
#include <string.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_image.h>

#define ALIGN_LEFT 0
#define ALIGN_CENTER 1
#define ALIGN_RIGHT 2

#define FORMAT_TEXT 0
#define FORMAT_IMAGE 1

#define DEFAULT_BG "#222222"
#define DEFAULT_FG "#999999"
#define DEFAULT_FONT "-*-terminus-medium-*-*-*-12-*-*-*-*-*-*-*"
#define X 0
#define Y 0
#define W 1280
#define H 14

std::vector<xcb_pixmap_t> pixmaps;

xcb_connection_t * connection;
xcb_screen_t * screen;
xcb_window_t window;
xcb_font_t font;
xcb_colormap_t colormap;

bool running = true;

uint32_t fg;
uint32_t bg;
int text_alignment = ALIGN_LEFT;

uint32_t default_fg;
uint32_t default_bg;

void add_text_pixmap(const std::string & string);
void add_bitmap_pixmap(const std::string & filename);
void add_spacer_pixmap(int space);
void draw_format_list();
void set_struts();
xcb_pixmap_t create_pixmap(int width, uint32_t bg);
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

    set_struts();

    xcb_map_window(connection, window);

    xcb_flush(connection);
}

void * draw_loop(void * arg)
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

void process_input(const std::string & input)
{
    size_t offset = 0;
    size_t marker = 0;

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
            pixmaps.clear();
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
            add_text_pixmap(string);
            std::cout << "string: " << string << "\n";
        }

        size_t arg_begin, arg_end;
        arg_begin = input.find('(', marker);
        if (arg_begin == std::string::npos)
        {
            std::cout << "Error in formatting string: couldn't find opening (\n";
            pixmaps.clear();
            return;
        }
        arg_end = input.find(')', arg_begin);
        if (arg_end == std::string::npos)
        {
            std::cout << "Error in formatting string: couldn't find closing )\n";
            pixmaps.clear();
            return;
        }

        std::string command = input.substr(marker + 1, arg_begin - marker - 1);
        std::string arg = input.substr(arg_begin + 1, arg_end - arg_begin - 1);

        std::cout << "command: " << command << ", arg: " << arg << "\n";

        if (command == "p")
        {
            int space = atoi(arg.c_str());
            add_spacer_pixmap(space);
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
            add_bitmap_pixmap(arg);
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
        add_text_pixmap(input.substr(offset));
        std::cout << "string: " << input.substr(offset) << "\n";
    }
    
    draw_format_list();
    pixmaps.clear();
}

void add_text_pixmap(const std::string & string)
{
    assert(string.size() > 0);

    uint32_t mask;
    uint32_t values[3];
    xcb_gcontext_t gc = xcb_generate_id(connection);

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

    xcb_pixmap_t pixmap = create_pixmap(text_extents->overall_width, bg);

    xcb_get_geometry_cookie_t geometry_cookie = xcb_get_geometry(connection, pixmap);

    mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND | XCB_GC_FONT;
    values[0] = fg;
    values[1] = bg;
    values[2] = font;

    xcb_create_gc(
        connection,
        gc,
        pixmap,
        mask,
        values
    );

    xcb_get_geometry_reply_t * geometry = xcb_get_geometry_reply(connection, geometry_cookie, NULL);

    xcb_image_text_8(
        connection,
        string.size(),
        pixmap,
        gc,
        0, (geometry->height + text_extents->font_ascent + text_extents->font_descent) / 2 - text_extents->font_descent,
        string.c_str()
    );

    pixmaps.push_back(pixmap);
}

void add_bitmap_pixmap(const std::string & filename)
{
    xcb_pixmap_t image_pixmap = 0;
    std::ifstream file(filename.c_str());
    std::string data;
    int width = 0;
    int height = 0;
    while (file.good())
    {
        file >> data;
        if (data == "#define")
        {
            file >> data;
            if (data.substr(data.size() - 5) == "width")
            {
                file >> width;
            }
            else if (data.substr(data.size() - 6) == "height")
            {
                file >> height;
            }
        }
        else if (data == "{")
        {
            assert(height > 0);
            assert(width > 0);
            int bitmap_size = ((width / 8) + (width % 8 == 0 ? 0 : 1)) * height;
            uint8_t bitmap[bitmap_size];
            int bitmap_position = 0;
            file >> data;
            while (data != "};" && bitmap_position < bitmap_size)
            {
                if (data.at(data.size() - 1) == ',')
                {
                    data = data.substr(0, data.size() - 1);
                }
                bitmap[bitmap_position] = strtol(data.c_str(), NULL, 16);
                bitmap_position++;
                file >> data;
            }
            assert(bitmap_position == bitmap_size);
            image_pixmap = xcb_create_pixmap_from_bitmap_data(
                connection,
                window,
                bitmap,
                width, height,
                screen->root_depth,
                fg,
                bg,
                NULL
            );
        }
    }
    assert(image_pixmap);

    xcb_pixmap_t pixmap = create_pixmap(width, bg);

    xcb_get_geometry_cookie_t geometry_cookie = xcb_get_geometry_unchecked(connection, pixmap);

    xcb_gcontext_t gc = xcb_generate_id(connection);
    xcb_create_gc(
        connection,
        gc,
        pixmap,
        0,
        NULL
    );
    xcb_get_geometry_reply_t * geometry = xcb_get_geometry_reply(connection, geometry_cookie, NULL);
    xcb_copy_area(
        connection,
        image_pixmap,
        pixmap,
        gc,
        0, 0,
        0, (geometry->height - height) / 2,
        width, height
    );

    pixmaps.push_back(pixmap);
}

void add_spacer_pixmap(int width)
{
    pixmaps.push_back(create_pixmap(width, bg));
}

xcb_pixmap_t create_pixmap(int width, uint32_t bg)
{
    xcb_get_geometry_cookie_t geometry_cookie = xcb_get_geometry_unchecked(
        connection,
        window
    );

    xcb_pixmap_t pixmap = xcb_generate_id(connection);
    xcb_gcontext_t gc = xcb_generate_id(connection);
    uint32_t mask = XCB_GC_FOREGROUND;
    uint32_t value = bg;

    xcb_get_geometry_reply_t * geometry = xcb_get_geometry_reply(
        connection,
        geometry_cookie,
        NULL
    );

    xcb_create_pixmap(
        connection,
        screen->root_depth,
        pixmap,
        window,
        width, geometry->height
    );

    xcb_create_gc(
        connection,
        gc,
        pixmap,
        mask,
        &value
    );

    xcb_rectangle_t rectangle;
    rectangle.x = 0;
    rectangle.y = 0;
    rectangle.width = width;
    rectangle.height = geometry->height;

    xcb_poly_fill_rectangle(
        connection,
        pixmap,
        gc,
        1,
        &rectangle
    );

    return pixmap;
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
    std::cout << "draw pixmap list, size: " << pixmaps.size() << "\n";

    xcb_get_geometry_cookie_t window_geometry_cookie = xcb_get_geometry_unchecked(connection, window);

    uint32_t mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND;
    uint32_t values[2];
    values[0] = default_fg;
    values[1] = default_bg;

    xcb_gcontext_t gc = xcb_generate_id(connection);

    xcb_create_gc(
        connection,
        gc,
        window,
        mask,
        values
    );

    xcb_get_geometry_reply_t * window_geometry = xcb_get_geometry_reply(connection, window_geometry_cookie, NULL);

    xcb_clear_area(
        connection,
        1,
        window,
        window_geometry->x, window_geometry->y,
        window_geometry->width, window_geometry->height
    );

    int x;
    if (text_alignment == ALIGN_LEFT)
    {
        x = 0;
    }
    else if (text_alignment == ALIGN_CENTER)
    {
        x = window_geometry->width / 2;
        std::vector<xcb_get_geometry_cookie_t> cookies;
        for (std::vector<xcb_pixmap_t>::const_iterator pixmap = pixmaps.begin(); pixmap != pixmaps.end(); ++pixmap)
        {
            cookies.push_back(xcb_get_geometry_unchecked(connection, *pixmap));
        }
        for(std::vector<xcb_get_geometry_cookie_t>::const_iterator cookie = cookies.begin(); cookie != cookies.end(); ++cookie)
        {
            x -= xcb_get_geometry_reply(connection, *cookie, NULL)->width / 2;
        }
    }
    else if (text_alignment == ALIGN_RIGHT)
    {
        x = window_geometry->width;
        std::vector<xcb_get_geometry_cookie_t> cookies;
        for (std::vector<xcb_pixmap_t>::const_iterator pixmap = pixmaps.begin(); pixmap != pixmaps.end(); ++pixmap)
        {
            cookies.push_back(xcb_get_geometry_unchecked(connection, *pixmap));
        }
        for(std::vector<xcb_get_geometry_cookie_t>::const_iterator cookie = cookies.begin(); cookie != cookies.end(); ++cookie)
        {
            x -= xcb_get_geometry_reply(connection, *cookie, NULL)->width;
        }
    }
    else
    {
        std::cout << "Unknown alignment: " << text_alignment << "\n";
        assert(false);
    }
    for (std::vector<xcb_pixmap_t>::const_iterator pixmap = pixmaps.begin(); pixmap != pixmaps.end(); ++pixmap)
    {
        xcb_get_geometry_cookie_t geometry_cookie = xcb_get_geometry_unchecked(connection, *pixmap);
        xcb_get_geometry_reply_t * geometry = xcb_get_geometry_reply(connection, geometry_cookie, NULL);
        xcb_copy_area(
            connection,
            *pixmap,
            window,
            gc,
            0, 0,
            x, (window_geometry->height - geometry->height) / 2,
            geometry->width, geometry->height
        );
        x += geometry->width;
    }
    xcb_flush(connection);
}

void set_struts()
{
    std::string property_name("_NET_WM_STRUT_PARTIAL");
    uint32_t strut[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    
    xcb_get_geometry_cookie_t geometry_cookie = xcb_get_geometry(connection, window);
    xcb_intern_atom_cookie_t property_cookie = xcb_intern_atom_unchecked(
        connection,
        0,
        property_name.size(),
        property_name.c_str()
    );

    xcb_get_geometry_reply_t * geometry = xcb_get_geometry_reply(
        connection,
        geometry_cookie,
        NULL
    );

    if (geometry->height == 0)
    {
        std::cout << "Invalid window geometry for setting struts: Window must have a non-zero height\n";
        return;
    }

    if (geometry->y == 0)
    {
        strut[2] = geometry->height;
        strut[8] = geometry->x;
        strut[9] = geometry->x + geometry->width - 1;
    }
    else if ((geometry->y + geometry->height)== screen->height_in_pixels)
    {
        strut[3] = geometry->height;
        strut[10] = geometry->x;
        strut[11] = geometry->x + geometry->width - 1;
    }
    else
    {
        std::cout << "Invalid window geometry for setting struts: Window must be on top or bottom of the screen\n";
        return;
    }

    xcb_intern_atom_reply_t * property = xcb_intern_atom_reply(
        connection,
        property_cookie,
        NULL
    );
    xcb_change_property(
        connection,
        XCB_PROP_MODE_REPLACE,
        window,
        property->atom,
        CARDINAL,
        32,
        12,
        strut
    );
}

