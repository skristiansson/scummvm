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

#include "backends/graphics/surfacesdl/surfacesdl20-graphics.h"

void SurfaceSdl20GraphicsManager::setColors(SDL_Surface *surface, SDL_Color *colors, int firstcolor, int ncolors) {
	if (surface->format->palette)
		SDL_SetPaletteColors(surface->format->palette, colors, firstcolor, ncolors);
}

void SurfaceSdl20GraphicsManager::setAlpha(SDL_Surface *surface,  Uint32 flag, Uint8 alpha) {
	SDL_SetSurfaceAlphaMod(surface, 255);
	if (!flag)
		SDL_SetSurfaceBlendMode(_screen, SDL_BLENDMODE_NONE);
}

void SurfaceSdl20GraphicsManager::setColorKey(SDL_Surface *surface,  int flag, Uint32 key) {
	SDL_SetColorKey(surface, SDL_TRUE, key);
}

void SurfaceSdl20GraphicsManager::blitToHwScreen() {
	int pitch;
	void *pixels;

	SDL_LockTexture(_hwtexture, NULL, &pixels, &pitch);
	SDL_ConvertPixels(_hwscreen->w, _hwscreen->h, _hwscreen->format->format,
		_hwscreen->pixels, _hwscreen->pitch, SDL_PIXELFORMAT_RGBA8888, pixels, pitch);
	SDL_UnlockTexture(_hwtexture);

	SDL_RenderClear(_hwrenderer);
	SDL_RenderCopy(_hwrenderer, _hwtexture, NULL, NULL);
	SDL_RenderPresent(_hwrenderer);
}

void SurfaceSdl20GraphicsManager::createHwScreen() {
	_hwwindow = SDL_CreateWindow("ScummVM",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		_videoMode.hardwareWidth, _videoMode.hardwareHeight,
		_videoMode.fullscreen ?
			(SDL_WINDOW_FULLSCREEN | SDL_WINDOW_SHOWN) :
			SDL_WINDOW_SHOWN);
	assert(_hwwindow);
	_hwrenderer = SDL_CreateRenderer(_hwwindow, -1, 0);
	assert(_hwrenderer);
	SDL_RenderSetLogicalSize(_hwrenderer, _videoMode.hardwareWidth, _videoMode.hardwareHeight);
	_hwtexture = SDL_CreateTexture(_hwrenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, _videoMode.hardwareWidth, _videoMode.hardwareHeight);
	_hwscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.hardwareWidth, _videoMode.hardwareHeight, 16, 0, 0, 0, 0);
}

void SurfaceSdl20GraphicsManager::destroyHwScreen() {
	if (_hwtexture) {
		SDL_DestroyTexture(_hwtexture);
		_hwtexture = NULL;
	}

	if (_hwrenderer) {
		SDL_DestroyRenderer(_hwrenderer);
		_hwrenderer = NULL;
	}

	if (_hwwindow) {
		SDL_DestroyWindow(_hwwindow);
		_hwwindow = NULL;
	}

	if (_hwscreen) {
		SDL_FreeSurface(_hwscreen);
		_hwscreen = NULL;
	}
}
