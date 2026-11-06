/*
 * Copyright (C) 2013-2026 iceman50
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

#include "stdafx.h"
#include "Dialog.h"
#include "Plugin.h"
#include "resource.h"

#include <pluginsdk/Config.hpp>
#include <pluginsdk/Core.hpp>
#include <pluginsdk/Util.hpp>

using dcapi::Config;
using dcapi::Core;
using dcapi::Util;

HINSTANCE Dialog::instance;
Dialog* dlg = nullptr;

Dialog::Dialog() :
	iconSm(nullptr),
	iconLg(nullptr),
	hwnd(nullptr)
{
	dlg = this;
}

Dialog::~Dialog() {
	close();

	dlg = nullptr;
}

void Dialog::create(HWND parent) {
	if(hwnd) {
		::ShowWindow(hwnd, SW_RESTORE);
		::SetForegroundWindow(hwnd);
		return;
	}
	if(!parent) parent = ::GetActiveWindow();
	::CreateDialog(instance, MAKEINTRESOURCE(IDD_MEDIAPLAYERDLG), parent, DialogProc);
	if(!hwnd) {
		TCHAR buf[256] = {};
		::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, buf, 256, 0);
		::MessageBox(parent, buf, _T("Error creating the MediaPlayer plugin's dialog"), MB_OK);
	}
}

void Dialog::saveConfig() {
	TCHAR waDest[2048] = {};
	::GetDlgItemText(hwnd, IDC_WINAMPCFG, waDest, static_cast<int>(sizeof(waDest) / sizeof(*waDest)));
	string waFormat = Util::fromT(waDest);

	if(waFormat.empty()) {
		const auto& defStr = "NP > %[title] [ %[elapsed] of %[length] ]";
		Config::setConfig("WAFormat", defStr); 
	} else {
		Config::setConfig("WAFormat", waFormat);
	}
}

void Dialog::helpBox(helpType type) {
	auto helpF = [](const string& helpStr) {
		::MessageBox(0, Util::toT(helpStr).c_str(),
			_T("MediaPlayer Help"), MB_OK);
	};
	
	switch (type) {
	case WINAMP:
		{
			helpF(MediaPlayers::WAHelp()); break;
		}
	case MPC:
		{
			helpF(MediaPlayers::MPCHelp()); break;
		}
	}
	 
}

void Dialog::close() {
	if(hwnd) {
		const auto window = hwnd;
		hwnd = nullptr;
		::DestroyWindow(window);
	}
}

INT_PTR CALLBACK Dialog::DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM) {
	switch(uMsg) {
	case WM_INITDIALOG:
		{
			const auto& icoPath = Util::toT(Config::getInstallPath() + "MediaPlayer.ico");
			dlg->iconSm = (HICON)::LoadImage(0, icoPath.c_str(), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_LOADFROMFILE);
			dlg->iconLg = (HICON)::LoadImage(0, icoPath.c_str(), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR | LR_LOADFROMFILE);

			try {
				::SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)dlg->iconSm);
				::SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)dlg->iconLg);
			} catch(...) { }

			dlg->hwnd = hWnd;
			return dlg->init();
		}
	case WM_COMMAND:
		{
			dlg->command(wParam);
			break;
		}
 	case WM_CLOSE:
		{
			if (dlg != nullptr) { dlg->close(); }
			break;
		}
	}
	if(uMsg == WM_NCDESTROY && dlg) {
		dlg->hwnd = nullptr;
		if(dlg->iconSm) ::DestroyIcon(dlg->iconSm);
		if(dlg->iconLg) ::DestroyIcon(dlg->iconLg);
		dlg->iconSm = dlg->iconLg = nullptr;
	}
	return FALSE;
}

BOOL Dialog::init() {
	::SendDlgItemMessage(hwnd, IDC_WINAMPCFG, EM_LIMITTEXT, 2047, 0);
	::SetDlgItemText(hwnd, IDC_WINAMPCFG,  Util::toT(Config::getConfig("WAFormat")).c_str());

	return TRUE;
}

void Dialog::command(WPARAM wParam) {
	switch(LOWORD(wParam)) {
	case IDOK:
		{
			saveConfig();
			close();
			break;
		}
	case IDCANCEL:
		{
			::SendMessage(hwnd, WM_CLOSE, 0, 0);
			break;
		}
	case IDC_WAHELP:
		{
			helpBox(WINAMP);
			break;
		}
	}
}
