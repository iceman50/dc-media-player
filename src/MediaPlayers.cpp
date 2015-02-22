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
 
#include "MediaPlayers.h"
#include "MediaUtil.h"
#include "Winamp.h"

#include <pluginsdk/Config.hpp>
#include <pluginsdk/Util.hpp>

#include <algorithm>

using dcapi::Config;
using dcapi::Util;

using std::max;
using std::min;

MediaPlayers::MediaPlayers() {
}

MediaPlayers::~MediaPlayers() {
}

string MediaPlayers::getWinampSpam(bool& status) {
	HWND hwndWinamp = ::FindWindow(_T("Winamp v1.x"), NULL);
	string result;
	if (hwndWinamp) {
		StringMap params;
		int waVersion = ::SendMessage(hwndWinamp, WM_USER, 0, IPC_GETVERSION),
			majorVersion, minorVersion;
		majorVersion = waVersion >> 12;
		if (((waVersion & 0x00F0) >> 4) == 0) {
			minorVersion = ((waVersion & 0x0f00) >> 8) * 10 + (waVersion & 0x000f);
		} else {
			minorVersion = ((waVersion & 0x00f0) >> 4) * 10 + (waVersion & 0x000f);
		}
		params["version"] = MediaUtil::toString(majorVersion + minorVersion / 100.0);
		int state = ::SendMessage(hwndWinamp, WM_USER, 0, IPC_ISPLAYING);
		switch (state) {
			case 0: params["state"] = "stopped";
				break;
			case 1: params["state"] = "playing";
				break;
			case 3: params["state"] = "paused";
		}
		TCHAR titleBuffer[2048];
		int buffLength = sizeof(titleBuffer);
		::GetWindowText(hwndWinamp, titleBuffer, buffLength);
		string title = Util::fromT(titleBuffer);
		params["rawtitle"] = title;
		// there's some winamp bug with scrolling. wa5.09x and 5.1 or something.. see:
		// http://forums.winamp.com/showthread.php?s=&postid=1768775#post1768775
		int starpos = (int)title.find("***");
		if (starpos >= 1) {
			string firstpart = title.substr(0, starpos - 1);
			if (firstpart == title.substr(title.size() - firstpart.size(), title.size())) {
				// fix title
				title = title.substr(starpos, title.size());
			}
		}
		// fix the title if scrolling is on, so we need to put the asterisks at the end of it
		string titletmp = title.substr(title.find("***") + 2, title.size());
		title = titletmp + title.substr(0, title.size() - titletmp.size());
		title = title.substr(title.find('.') + 2, title.size());
		if (title.rfind('-') != string::npos) {
			params["title"] = title.substr(0, title.rfind('-') - 1);
		}
		int curPos = ::SendMessage(hwndWinamp, WM_USER, 0, IPC_GETOUTPUTTIME);
		int length = ::SendMessage(hwndWinamp, WM_USER, 1, IPC_GETOUTPUTTIME);
		if (curPos == -1) {
			curPos = 0;
		} else {
			curPos /= 1000;
		}
		int intPercent;
		if (length > 0 ) {
			intPercent = curPos * 100 / length;
		} else {
			length = 0;
			intPercent = 0;
		}
		params["percent"] = MediaUtil::toString(intPercent) + "%";
		params["elapsed"] = MediaUtil::formatSeconds(curPos);
		params["length"] = MediaUtil::formatSeconds(length);
		if(length>0) {
			int numFront = min(max(intPercent / 10, 0), 10),
				numBack = min(max(10 - 1 - numFront, 0), 10);
			string inFront = string(numFront, '-'),
				inBack = string(numBack, '-');
			string barChar;
			if(state) {
				barChar = "|";
			} else {
				barChar = "-";
			}
			params["bar"] = "[" + inFront + barChar + inBack + "]";
			params["progress"] = "[" + inFront + barChar + inBack + "]";
		}
		int waSampleRate = ::SendMessage(hwndWinamp, WM_USER, 0, IPC_GETINFO),
			waBitRate = ::SendMessage(hwndWinamp, WM_USER, 1, IPC_GETINFO),
			waChannels = ::SendMessage(hwndWinamp, WM_USER, 2, IPC_GETINFO);
		params["bitrate"] = MediaUtil::toString(waBitRate) + "kbps";
		params["sample"] = MediaUtil::toString(waSampleRate) + "kHz";
		string waChannelName;
		switch (waChannels) {
			case 2:
				waChannelName = "stereo";
				break;
			case 6:
				waChannelName = "5.1 surround";
				break;
			default:
				waChannelName = "mono";
		}
		params["channels"] = waChannelName;
		result = MediaUtil::formatParams(Config::getConfig("WAFormat"), params);
	} else {
		status = true;
		result = "Winamp is not running";
	}
	return result;
}

string MediaPlayers::getMPCSpam(bool& status) {
	string result;
	TCHAR wTitle[1024];
	//Unicode only ...
	HWND hwnd = ::FindWindow(_T("MediaPlayerClassicW"), NULL);

	if(hwnd) {
		StringMap params;
		::GetWindowText(hwnd, (LPWSTR)wTitle, 1024);
		string title = Util::fromT(wTitle);
		if(!title.find(" - Media Player Classic")) { 
			status = true;
			result = "Can't retrieve MPC pattern"; 
		}
		//TODO add the rest of the param formatting
		params["title"] = title;
		//result = MediaUtil::formatParams(Config::getConfig("MPCFormat"), params);
		result = title;
	} else {
		status = true;
		result = "MPC is not running";
	}
	return result;
}

const string MediaPlayers::WAHelp() {
	const string buf =
		"Winamp media broadcast\r\n"
		"- %[version]	Numerical Version (ex: 2.91)\r\n"
		"- %[state]	Playing state (ex: stopped/paused/playing)\r\n"		
		"- %[title]		Window title from Winamp.\r\n"
		"- %[rawtitle]	Window title from Winamp (if %[title] is not working properly)\r\n"
		"- %[percent]	Percentage (ex. 40%)\r\n"
		"- %[length]	Length in minutes:seconds (ex: 04:09)\r\n"
		"- %[elapsed]	Time elapsed in minutes:seconds (ex. 03:51)\r\n"
		"- %[progress]	ASCII progress bar, 12 characters wide\r\n"
		"- %[bar]		Same as %[progress] \r\n"
		"- %[bitrate]	Bitrate (ex. 128kbps)\r\n"
		"- %[sample]	Sample frequency (ex. 22kHz)\r\n"
		"- %[channels]	Number of channels (ex. stereo / mono)\r\n"
		"Empty = Default String -> NP > %[title] [ %[elapsed] of %[length] ]";
	return buf;
}

const string MediaPlayers::MPCHelp() {
	const string buf =
		//TODO Finish when the parameters are all added
		"MPC broadcast\r\n"
		"- %[title]		Show the raw title of the media playing";
	return buf;
}
