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
#include "backends/fs/zip/zip-fs.h"

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
	if (path.matchString("*.scummz*")) {
		Common::String zipFile = normalizePath(path, '/');
		Common::String pathInZip;

		// Split 'path' into a path to the zip file and a path within the zip
		while (!zipFile.hasSuffix(".scummz")) {
			pathInZip = lastPathComponent(zipFile, '/') + '/' + pathInZip;
			zipFile = ZipFilesystemNode::getParentPath(zipFile, '/');
		}

		if (pathInZip.lastChar() == '/')
			pathInZip.deleteLastChar();

		return new ZipFilesystemNode(zipFile, pathInZip, '\\');
	}

	return new WP81FilesystemNode(path);
}
