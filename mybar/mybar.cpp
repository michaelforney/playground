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
#include <pthread.h>
#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char * argv[])
{
    pthread_t input_thread;
    pthread_t draw_thread;

    pthread_attr_t input_attr;

    pthread_attr_init(&input_attr);
    pthread_attr_setdetachstate(&input_attr, PTHREAD_CREATE_JOINABLE);

    draw_init();

    std::cout << "Creating input thread: " << pthread_create(&input_thread, &input_attr, input_loop, NULL) << "\n";
    std::cout << "Creating draw thread: " << pthread_create(&draw_thread, NULL, draw_loop, NULL) << "\n";

    pthread_attr_destroy(&input_attr);

    void * test;
    pthread_join(input_thread, &test);
    pthread_cancel(draw_thread);

    draw_cleanup();

    return 0;
}

