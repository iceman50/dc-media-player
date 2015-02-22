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

#include "stdafx.h"
#include "Plugin.h"
#include "MediaPlayers.h"

#include <pluginsdk/Config.h>
#include <pluginsdk/Core.h>
#include <pluginsdk/Hooks.h>
#include <pluginsdk/Hubs.h>
#include <pluginsdk/Logger.h>
#include <pluginsdk/UI.h>
#include <pluginsdk/Util.h>

using dcapi::Config;
using dcapi::Core;
using dcapi::Hooks;
using dcapi::Hubs;
using dcapi::Logger;
using dcapi::UI;
using dcapi::Util;

static const string commandName = "Configure media broadcasts";

Plugin::Plugin() {
}

Plugin::~Plugin() {
	clearHooks();

	if(UI::handle()) {
		UI::removeCommand(commandName);
	}
}

Plugin* instance;

Bool DCAPI Plugin::main(PluginState state, DCCorePtr core, dcptr_t) {
	switch(state) {
	case ON_INSTALL:
	case ON_LOAD:
	case ON_LOAD_RUNTIME:
		{
			instance = new Plugin();
			return instance->onLoad(core, state == ON_INSTALL, state == ON_INSTALL || state == ON_LOAD_RUNTIME) ? True : False;
		}
	case ON_UNINSTALL:
	case ON_UNLOAD:
		{
			delete instance;
			instance = nullptr;
			return True;
		}
	case ON_CONFIGURE: 
		{
			instance->dialog.create();
			return True;
		}
	default:
		{
			return False;
		}
	}
}

void Plugin::addHooks() {
	Hooks::UI::onChatCommand([this](HubDataPtr hHub, CommandDataPtr cmd, bool&) { return onChatCommand(hHub, cmd); });
	Hooks::UI::onChatCommandPM([this](UserDataPtr hUser, CommandDataPtr cmd, bool&) { return onChatCommandPM(hUser, cmd); });
}

void Plugin::clearHooks() {
	Hooks::clear();
}

void Plugin::start(bool showDialog) {
	addHooks();
	if(showDialog) { instance->dialog.create(); }
}

void Plugin::close(bool isDialog) {
	clearHooks();
	if(isDialog) { instance->dialog.close(); }
}

bool Plugin::onLoad(DCCorePtr core, bool install, bool runtime) {
	Core::init(core);
	if(!Config::init(PLUGIN_GUID) || !Hooks::init() || !Hubs::init() || !Logger::init() || !UI::init(PLUGIN_GUID) || !Util::init()) {
		return false;
	}

	if(install) {
		Config::setConfig("WAFormat", "NP > %[title] [ %[elapsed] of %[length] ]");
		Config::setConfig("MPCFormat", "NP > %[title]");

		Logger::log("The MediaPlayer plugin has been installed; check the \"" + commandName + "\" command.");
	}

	if(runtime) {
		// If the plugin doesn't support being enabled at runtime, cancel its loading here.
	}

	start(install);
	UI::addCommand(commandName, [] { instance->onSwitched();  }, Config::getInstallPath() + "MediaPlayer.ico");

	return true;
}

void Plugin::onSwitched() {
	dialog.exist() ? dialog.close() : dialog.create();
}

bool Plugin::onChatCommand(HubDataPtr hub, CommandDataPtr cmd) {
	string result = "";
	bool failed = false;
	
	if(parseCommand(cmd->command, cmd->params, result, failed)) {
		if(!failed) {
			Bool thirdPerson = False;
			if(strnicmp(result.c_str(), "/me ", 4) == 0) {
				thirdPerson = True;
				result = result.substr(4);
			}
			Hubs::handle()->send_message(hub, result.c_str(), thirdPerson);
		} else {
			Hubs::handle()->local_message(hub, result.c_str(), MSG_SYSTEM);
		}
		return true;
	} else {
		return false;
	}
}

bool Plugin::onChatCommandPM(UserDataPtr user, CommandDataPtr cmd) {
	string result = "";
	bool failed = false;
	
	if(parseCommand(cmd->command, cmd->params, result, failed)) {
		if(!failed) {
			Bool thirdPerson = False;
			if(strnicmp(result.c_str(), "/me ", 4) == 0) {
				thirdPerson = True;
				result = result.substr(4);
			}
			Hubs::handle()->send_private_message(user, result.c_str(), thirdPerson);
		} else {
			return false; // Will let it be for now until a better solution comes up
		}
		return true;
	} else {
		return false;
	}
}

bool Plugin::parseCommand(const char* cmd, const char* param, string& result, bool& status) {
	if(stricmp(cmd, "help") == 0) {
		if(stricmp(param, "plugins") == 0) {
			result =
				"\t\t\t Help: MediaPlayer plugin \t\t\t\n"
				"\t /w, /wa, /winamp \t\t\t\t Broadcast what's currently playing in Winamp\n"
				"\t /mpc \t\t\t\t Broadcast what's currently playing in MPC";

			status = true;
			return true;
		}
	} else if((stricmp(cmd, "w") == 0) || (stricmp(cmd, "winamp") == 0) || (stricmp(cmd, "wa") == 0)) {
		result = MediaPlayers::getWinampSpam(status);
		return true;
	} else if ((stricmp(cmd, "mpc") == 0)) {
		result = MediaPlayers::getMPCSpam(status);
		return true;
	}
	return false;
}
