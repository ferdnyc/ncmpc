/* ncmpc (Ncurses MPD Client)
 * (c) 2004-2019 The Music Player Daemon Project
 * Project homepage: http://musicpd.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef NCMPC_H
#define NCMPC_H

#include "AsioServiceFwd.hxx"

#ifdef HAVE_GETMOUSE
#include <curses.h>
#endif

enum class Command : unsigned;
struct Point;
class ScreenManager;
extern ScreenManager *screen;

void begin_input_event();
void end_input_event();

/**
 * @return false if the application shall quit
 */
bool
do_input_event(boost::asio::io_service &io_service, Command cmd);

#ifdef HAVE_GETMOUSE
void
do_mouse_event(Point p, mmask_t bstate);
#endif

#endif /* NCMPC_H */
