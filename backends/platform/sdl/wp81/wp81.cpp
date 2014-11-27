/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include <Windows.h>

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "backends/platform/sdl/wp81/wp81.h"
#include "backends/events/wp81sdl/wp81sdl-events.h"
#include "backends/fs/wp81/wp81-fs.h"
#include "backends/fs/wp81/wp81-fs-factory.h"
#include "backends/mixer/sdl13/sdl13-mixer.h"

using namespace Windows::Storage;

OSystem_WP81::OSystem_WP81() : _virtualKbd(false) {
	_fsFactory = new WP81FilesystemFactory();
}

void OSystem_WP81::initBackend() {
	StorageFolder ^installedLocation = Windows::ApplicationModel::Package::Current->InstalledLocation;
	StorageFolder ^local = ApplicationData::Current->LocalFolder;

	WP81FilesystemNode::createDir(local, "saves");

	ConfMan.set("vkeybdpath", Common::String(WP81FilesystemNode::toAscii(installedLocation->Path->Data())) + "\\backends\\vkeybd\\packs\\vkeybd_default\\");
	ConfMan.set("themepath", Common::String(WP81FilesystemNode::toAscii(installedLocation->Path->Data())) + "\\gui\\themes\\");
	ConfMan.set("savepath", Common::String(WP81FilesystemNode::toAscii(local->Path->Data())) + "\\saves\\");
	ConfMan.registerDefault("fullscreen", true);
	ConfMan.registerDefault("aspect_ratio", true);

	if (_mixer == 0) {
		_mixerManager = new Sdl13MixerManager();

		_mixerManager->init();
	}

	if (_eventSource == 0)
		_eventSource = new WP81SdlEventSource();

	SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
	SDL_SetHint(SDL_HINT_WINRT_HANDLE_BACK_BUTTON, "1");

	OSystem_SDL::initBackend();
}

void OSystem_WP81::logMessage(LogMessageType::Type type, const char *message) {
	FILE *output = 0;

	if (type == LogMessageType::kInfo || type == LogMessageType::kDebug)
		output = stdout;
	else
		output = stderr;

	fputs(message, output);
	fflush(output);

	OutputDebugStringA(message);
}

Common::String OSystem_WP81::getDefaultConfigFileName() {
	StorageFolder ^local = ApplicationData::Current->LocalFolder;

	return Common::String(WP81FilesystemNode::toAscii(local->Path->Data())) + "\\scummvm.ini";
}
