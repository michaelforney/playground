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

#include <iostream>
#include <pthread.h>
#include <string>

void * input_loop(void * arg)
{
    std::string input;
    while(true)
    {
        std::getline(std::cin, input);
        if (std::cin.eof())
        {
            std::cout << "eof, exiting thread\n";
            pthread_exit(NULL);
        }
        std::cout << "size: " << input.size() << "\n";
        std::cout << "input: " << input << "\n";
        process_input(input);
    }
}

