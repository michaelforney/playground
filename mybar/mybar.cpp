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

#include "input.h"
#include "draw.h"

#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <pthread.h>
#include <cstdlib>
#include <string>

pthread_t input_thread;

int main(int argc, char * argv[])
{
    draw_init();

    input_thread = pthread_create(&input_thread, NULL, input_loop, NULL);
    draw_loop();

    draw_cleanup();

    return 0;
}

