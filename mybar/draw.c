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

#include <xcb/xcb.h>

#define ALIGN_LEFT 0
#define ALIGN_CENTER 1
#define ALIGN_RIGHT 2

#define TEXT_FORMAT_COUNT 512

typedef struct
{
    int position,
    const char * text,
    uint32_t fg,
    uint32_t bg,
} text_format;

void draw(const char * input)
{
    size_t length;
    int cursor = 0;
    int text_alignment = ALIGN_LEFT;
    uint32_t fg;
    uint32_t bg;

    text_format * text_format_list = NULL;
    int text_format_list_size = 0;
    int text_format_list_position = 0;

    length = strlen(input);

    char * pos = input;
    char * marker = NULL;

    cursor = 0;
    while((marker = strchr(pos, '^')) != NULL)
    {
        if (pos < marker)
        {
            if (text_format_list_position >= text_format_list_size)
            {
                text_format_list_size *= 2;
                text_format_list = realloc(text_format_list, sizeof(text_format) * text_format_list_size);
                assert(text_format_list);
            }
            text_format * format = text_format_list + text_format_list_position;

            format->text = malloc((marker - pos + 1) * sizeof(char));
            strncpy(format->text, pos, marker - pos);
            format->text[marker - pos] = '\0';
            format->fg = fg;
            format->bg = bg;
            formap->position = cursor;

            // TODO: how much does the cursor move?
            // cursor += ?

            pos = marker + 1;

            text_format_list_position++;
        }
        if(pos[0] == 'p')
        {
            assert(pos[1] == '(');
            cursor += strtol(pos + 2, &pos, 10);
            assert(pos[0] == ')');
            pos++;
        }
        else if(pos[0] == 'b' && pos[1] == 'g')
        {
            assert(pos[2] == '(');
            pos += 3;
            
        }

    }
}

void reset_text_buffer()
{
    text_buffer_pos = 0;
}

void draw_text(const char * string, size_t length)
{
    xcb_image_text_8(
        connection,
        length,
        window,
        gc,
        cursor, 10,
        string
    );
}

