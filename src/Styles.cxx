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

#include "Styles.hxx"
#include "BasicColors.hxx"
#include "CustomColors.hxx"
#include "i18n.h"
#include "util/RuntimeError.hxx"
#include "util/StringStrip.hxx"
#include "util/Compiler.h"

#ifdef ENABLE_COLORS
#include "Options.hxx"
#endif

#include <assert.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>

/**
 * Use the terminal's default color.
 *
 * @see init_pair(3ncurses)
 */
static constexpr short COLOR_NONE = -1;

/**
 * A non-standad magic value which means "inherit this color from the
 * parent style".
 */
static constexpr short COLOR_INHERIT = -2;

/**
 * A non-standad magic value which means "inherit attributes from the
 * parent style".
 */
static constexpr attr_t A_INHERIT = ~attr_t(0);

struct StyleData {
	/**
	 * A name which can be used to address this style from the
	 * configuration file.
	 */
	const char *const name;

#ifdef ENABLE_COLORS
	/**
	 * Inherit unspecified values from this style.  The special
	 * value #Style::DEFAULT means "don't inherit".
	 */
	const Style inherit;

	/**
	 * The foreground (text) color in "color" mode.
	 */
	short fg_color;

	/**
	 * The background (fill) color in "color" mode.
	 */
	short bg_color;

	/**
	 * The attributes in "color" mode.
	 */
	attr_t attr;
#endif

	/**
	 * The attributes in "mono" mode.
	 */
	const attr_t mono;

#ifndef ENABLE_COLORS
	constexpr StyleData(const char *_name, Style,
			    short, short, attr_t, attr_t _mono) noexcept
		:name(_name), mono(_mono) {}
#endif
};

#ifndef ENABLE_COLORS
constexpr
#endif
static StyleData styles[size_t(Style::END)] = {
	/* color pair = field name, color, mono */
	{
		nullptr, Style::DEFAULT,
		COLOR_NONE, COLOR_NONE, A_NORMAL,
		A_NORMAL,
	},
	{
		"title", Style::BACKGROUND,
		COLOR_WHITE, COLOR_BLUE, A_NORMAL,
		A_NORMAL,
	},
	{
		"title-bold", Style::TITLE,
		COLOR_YELLOW, COLOR_INHERIT, A_BOLD,
		A_BOLD,
	},
	{
		"line", Style::TITLE,
		COLOR_WHITE, COLOR_INHERIT, A_NORMAL,
		A_NORMAL,
	},
	{
		"line-bold", Style::LINE,
		COLOR_INHERIT, COLOR_INHERIT, A_BOLD,
		A_BOLD,
	},
	{
		"line-flags", Style::LINE,
		COLOR_GREEN, COLOR_INHERIT, A_BOLD,
		A_NORMAL,
	},
	{
		"list", Style::BACKGROUND,
		COLOR_WHITE, COLOR_INHERIT, A_NORMAL,
		A_NORMAL,
	},
	{
		"list-bold", Style::LIST,
		COLOR_INHERIT, COLOR_INHERIT, A_BOLD,
		A_BOLD,
	},
	{
		"progressbar", Style::STATUS,
		COLOR_WHITE, COLOR_INHERIT, A_BOLD,
		A_NORMAL,
	},
	{
		"progressbar-background", Style::PROGRESSBAR,
		COLOR_BLACK, COLOR_INHERIT, A_BOLD,
		A_NORMAL,
	},
	{
		"status-song", Style::BACKGROUND,
		COLOR_WHITE, COLOR_BLUE, A_NORMAL,
		A_NORMAL,
	},
	{
		"status-state", Style::STATUS,
		COLOR_GREEN, COLOR_INHERIT, A_BOLD,
		A_BOLD,
	},
	{
		"status-time", Style::STATUS_BOLD,
		COLOR_INHERIT, COLOR_INHERIT, A_NORMAL,
		A_NORMAL,
	},
	{
		"alert", Style::STATUS,
		COLOR_RED, COLOR_INHERIT, A_BOLD,
		A_BOLD,
	},
	{
		"browser-directory", Style::LIST,
		COLOR_YELLOW, COLOR_INHERIT, A_INHERIT,
		A_NORMAL,
	},
	{
		"browser-playlist", Style::LIST,
		COLOR_RED, COLOR_INHERIT, A_INHERIT,
		A_NORMAL,
	},
	{
		"background", Style::DEFAULT,
		COLOR_NONE, COLOR_BLACK, A_NORMAL,
		A_NORMAL,
	},
};

static constexpr auto &
GetStyle(Style style) noexcept
{
	return styles[size_t(style)];
}

#ifdef ENABLE_COLORS

gcc_pure
static Style
StyleByName(const char *name) noexcept
{
	for (size_t i = 1; i < size_t(Style::END); ++i)
		if (!strcasecmp(styles[i].name, name))
			return Style(i);

	return Style::END;
}

static void
colors_update_pair(Style style) noexcept
{
	auto &data = GetStyle(style);

	int fg = data.fg_color;
	for (Style i = style; fg == COLOR_INHERIT;) {
		i = GetStyle(i).inherit;
		assert(i != Style::DEFAULT);
		fg = GetStyle(i).fg_color;
	}

	int bg = data.bg_color;
	for (Style i = style; bg == COLOR_INHERIT;) {
		i = GetStyle(i).inherit;
		assert(i != Style::DEFAULT);
		bg = GetStyle(i).bg_color;
	}

	/* apply A_INHERIT (modifies the "attr" value, which is
	   irreversible) */
	for (Style i = style; data.attr == A_INHERIT;) {
		i = GetStyle(i).inherit;
		assert(i != Style::DEFAULT);
		data.attr = GetStyle(i).attr;
	}

	init_pair(short(style), fg, bg);
}

/**
 * Throws on error.
 */
static short
ParseBackgroundColor(const char *s)
{
	short color = ParseColorNameOrNumber(s);
	if (color >= 0)
		return color;

	if (!strcasecmp(s, "none"))
		return COLOR_NONE;

	throw FormatRuntimeError("%s: %s", _("Unknown color"), s);
}

/**
 * Throws on error.
 */
static void
ParseStyle(StyleData &d, const char *str)
{
	std::string copy(str);

	for (char *cur = strtok(&copy.front(), ","); cur != nullptr;
	     cur = strtok(nullptr, ",")) {
		cur = Strip(cur);
		char *slash = strchr(cur, '/');
		if (slash != nullptr) {
			const char *name = slash + 1;
			d.bg_color = ParseBackgroundColor(name);

			*slash = 0;

			if (*cur == 0)
				continue;
		}

		/* Legacy colors (brightblue,etc) */
		if (!strncasecmp(cur, "bright", 6)) {
			d.attr |= A_BOLD;
			cur += 6;
		}

		/* Colors */
		short b = ParseColorNameOrNumber(cur);
		if (b >= 0) {
			d.fg_color = b;
			continue;
		}

		if (!strcasecmp(cur, "none"))
			d.fg_color = COLOR_NONE;
		else if (!strcasecmp(cur, "grey") ||
			 !strcasecmp(cur, "gray")) {
			d.fg_color = COLOR_BLACK;
			d.attr |= A_BOLD;
		}

		/* Attributes */
		else if (!strcasecmp(cur, "standout"))
			d.attr |= A_STANDOUT;
		else if (!strcasecmp(cur, "underline"))
			d.attr |= A_UNDERLINE;
		else if (!strcasecmp(cur, "reverse"))
			d.attr |= A_REVERSE;
		else if (!strcasecmp(cur, "blink"))
			d.attr |= A_BLINK;
		else if (!strcasecmp(cur, "dim"))
			d.attr |= A_DIM;
		else if (!strcasecmp(cur, "bold"))
			d.attr |= A_BOLD;
		else
			throw FormatRuntimeError("%s: %s",
						 _("Unknown color"), str);
	}
}

void
ModifyStyle(const char *name, const char *value)
{
	const auto style = StyleByName(name);
	if (style == Style::END)
		throw FormatRuntimeError("%s: %s",
					 _("Unknown color field"), name);

	auto &data = GetStyle(style);

	if (style == Style::BACKGROUND) {
		/* "background" is a special style which all other
		   styles inherit their background color from; if the
		   user configures a color, it will be the background
		   color, but no attributes */
		data.bg_color = ParseBackgroundColor(value);
		return;
	}

	return ParseStyle(data, value);
}

void
ApplyStyles() noexcept
{
	if (has_colors()) {
		/* initialize color support */
		start_color();
		use_default_colors();
		/* define any custom colors defined in the configuration file */
		ApplyCustomColors();

		if (options.enable_colors) {
			for (size_t i = 1; i < size_t(Style::END); ++i)
				/* update the color pairs */
				colors_update_pair(Style(i));
		}
	} else if (options.enable_colors) {
		fprintf(stderr, "%s\n",
			_("Terminal lacks color capabilities"));
		options.enable_colors = false;
	}
}
#endif

void
SelectStyle(WINDOW *w, Style style) noexcept
{
	const auto &data = GetStyle(style);

#ifdef ENABLE_COLORS
	if (options.enable_colors) {
		/* color mode */
		wattr_set(w, data.attr, short(style), nullptr);
	} else {
#endif
		/* mono mode */
		(void)wattrset(w, data.mono);
#ifdef ENABLE_COLORS
	}
#endif
}
