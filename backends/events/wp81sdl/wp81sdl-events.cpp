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

#include "common/scummsys.h"

#include "backends/events/wp81sdl/wp81sdl-events.h"
#include "backends/platform/sdl/sdl.h"
#include "engines/engine.h"

#include "common/util.h"
#include "common/events.h"

bool WP81SdlEventSource::dispatchSDLEvent(SDL_Event &ev, Common::Event &event) {
#ifdef ENABLE_VKEYBD
	// Bring up the virtual keyboard when the back button is pressed.
	// SDL will turn the single event into a SDL_KEYDOWN/SDL_KEYUP sequence,
	// we choose to trigger on the SDL_KEYUP event here.
	if (ev.type == SDL_KEYDOWN && ev.key.keysym.scancode == SDL_SCANCODE_AC_BACK)
		return true;

	if (ev.type == SDL_KEYUP && ev.key.keysym.scancode == SDL_SCANCODE_AC_BACK) {
		event.type = Common::EVENT_VIRTUAL_KEYBOARD;
		return true;
	}
#endif
	// Fall back to the parent handler.
	return SdlEventSource::dispatchSDLEvent(ev, event);
}
