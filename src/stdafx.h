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

#ifndef PLUGINS_MEDIA_PLAYER_STDAFX_H
#define PLUGINS_MEDIA_PLAYER_STDAFX_H

#include <windows.h>
#include <tchar.h>

#include <cstdio>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <pluginsdk/PluginDefs.h>

#include "version.h"

using std::string;

#ifdef _UNICODE
typedef std::wstring tstring;
#else
typedef std::string tstring;
#endif

#if defined(_MSC_VER)
#pragma warning(disable : 4244) // LRESULT to int conversions in MediaPlayer.cpp
#pragma warning(disable : 4996) // deprecated functions _snprintf -> _snprintf_s
#define _LL(x) x##ll
#define _ULL(x) x##ull
#define I64_FMT "%I64d"
#define U64_FMT "%I64d"
#define snprintf _snprintf
#define snwprintf _snwprintf
#endif

typedef std::map<std::string, std::string> StringMap;
typedef StringMap::const_iterator StringMapIter;

typedef std::vector<std::string> StringList;
typedef StringList::const_iterator StringIter;

#endif
