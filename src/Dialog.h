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

#ifndef PLUGINS_MEDIA_PLAYER_DIALOG_H
#define PLUGINS_MEDIA_PLAYER_DIALOG_H

#include "stdafx.h"

#include "MediaPlayers.h"

#include <commctrl.h>
#include <windowsx.h>

class Dialog
{

public:
	Dialog();
	~Dialog();

	void create();
	void close();
	bool exist() { return hwnd; }

	static HINSTANCE instance;
private:
	enum helpType {
		WINAMP,
		MPC
	};

	void saveConfig();
	void helpBox(helpType type);

	HICON iconSm, iconLg;

	static INT_PTR CALLBACK DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM);

	// events called by DialogProc
	BOOL init();
	void command(WPARAM wParam);

	HWND hwnd;
};

#endif
