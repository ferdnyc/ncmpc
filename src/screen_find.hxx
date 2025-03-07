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

#ifndef NCMPC_SCREEN_FIND_H
#define NCMPC_SCREEN_FIND_H

enum class Command : unsigned;
class ScreenManager;
class ListWindow;
class ListRenderer;
class ListText;

/**
 * query user for a string and find it in a list window
 *
 * @param lw the list window to search
 * @param findcmd the search command/mode
 * @param callback_fn a function returning the text of a given line
 * @param callback_data a pointer passed to callback_fn
 * @return true if the command has been handled, false if not
 */
bool
screen_find(ScreenManager &screen, ListWindow &lw,
	    Command findcmd,
	    const ListText &text) noexcept;

/* query user for a string and jump to the entry
 * which begins with this string while the users types */
void
screen_jump(ScreenManager &screen, ListWindow &lw,
	    const ListText &text, const ListRenderer &renderer) noexcept;

#endif
