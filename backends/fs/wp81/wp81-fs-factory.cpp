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

// Disable symbol overrides for FILE
#define FORBIDDEN_SYMBOL_EXCEPTION_FILE

#include "backends/fs/wp81/wp81-fs-factory.h"
#include "backends/fs/wp81/wp81-fs.h"
#include "backends/fs/romfs/romfs-fs.h"

namespace Common {
DECLARE_SINGLETON(WP81FilesystemFactory);
}

AbstractFSNode *WP81FilesystemFactory::makeRootFileNode() const {
	return new WP81FilesystemNode();
}

AbstractFSNode *WP81FilesystemFactory::makeCurrentDirectoryFileNode() const {
	return new WP81FilesystemNode();
}

AbstractFSNode *WP81FilesystemFactory::makeFileNodePath(const Common::String &path) const {
	if (path.matchString("*.romfs*")) {
		Common::String romFile = normalizePath(path, '/');
		Common::String pathInRom;

		// Split 'path' into a path to the rom file and a path within the rom
		while (!romFile.hasSuffix(".romfs")) {
			pathInRom = lastPathComponent(romFile, '/') + '/' + pathInRom;
			romFile = RomfsFilesystemNode::getParentPath(romFile, '/');
		}

		if (pathInRom.lastChar() == '/')
			pathInRom.deleteLastChar();

		return new RomfsFilesystemNode(romFile, pathInRom, '/');
	}

	return new WP81FilesystemNode(path);
}
