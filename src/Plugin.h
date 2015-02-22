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

#ifndef PLUGINS_MEDIA_PLAYER_PLUGIN_H
#define PLUGINS_MEDIA_PLAYER_PLUGIN_H

#include "Dialog.h"
#include "version.h"

using std::string;

class Plugin
{
public:
	static Bool DCAPI main(PluginState state, DCCorePtr core, dcptr_t);

	bool parseCommand(const char* cmd, const char* param, string& result, bool& status);
	
private:
	Plugin();
	~Plugin();

	void addHooks();
	void clearHooks();

	void start(bool showDialog = true);
	void close(bool isDialog = true);	

	bool onLoad(DCCorePtr core, bool install, bool runtime);
	void onSwitched();
	
	bool onChatCommand(HubDataPtr hub, CommandDataPtr cmd);
	bool onChatCommandPM(UserDataPtr user, CommandDataPtr cmd);

	Dialog dialog;
};

#endif
