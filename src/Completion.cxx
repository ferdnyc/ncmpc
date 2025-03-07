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

#include "Completion.hxx"

#include <assert.h>

static bool
StartsWith(const std::string &haystack, const std::string &needle) noexcept
{
	return haystack.length() >= needle.length() &&
		std::equal(needle.begin(), needle.end(), haystack.begin());
}

Completion::Result
Completion::Complete(const std::string &prefix) const noexcept
{
	auto lower = list.lower_bound(prefix);
	if (lower == list.end() || !StartsWith(*lower, prefix))
		return {std::string(), {lower, lower}};

	auto upper = list.upper_bound(prefix);
	while (upper != list.end() && StartsWith(*upper, prefix))
		++upper;

	assert(upper != lower);

	auto m = std::mismatch(lower->begin(), lower->end(),
			       std::prev(upper)->begin()).first;

	return {{lower->begin(), m}, {lower, upper}};
}
