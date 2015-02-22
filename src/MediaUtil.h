/*
 * Copyright (C) 2013 iceman50
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef PLUGINS_MEDIA_PLAYER_MEDIA_UTIL_H
#define PLUGINS_MEDIA_PLAYER_MEDIA_UTIL_H

using std::string;

class MediaUtil
{
public:

	static string toString(uint16_t val) {
		char buf[8];
		snprintf(buf, sizeof(buf), "%u", (unsigned int)val);
		return buf;
	}

	static string toString(int32_t val) {
		char buf[16];
		snprintf(buf, sizeof(buf), "%d", val);
		return buf;
	}
	
	static string toString(double val) {
		char buf[16];
		snprintf(buf, sizeof(buf), "%0.2f", val);
		return buf;
	}

	static string formatSeconds(uint64_t aSec, bool supressHours = false) {
		char buf[64];

		if (!supressHours) {
			snprintf(buf, sizeof(buf), "%01lu:%02d:%02d", (unsigned long)(aSec / (60 * 60)), (int)((aSec / 60) % 60), (int)(aSec % 60));
		} else {
			snprintf(buf, sizeof(buf), "%02d:%02d", (int)(aSec / 60), (int)(aSec % 60));
		}
		return buf;
	}
	
	static string formatParams(const string& format, StringMap& params);
	static void tokenizeString(const string& str, const char* limiter, StringList& result);

};

#endif
