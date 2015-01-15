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

#include "common/system.h"
#include "common/list.h"
#include "backends/fs/fs-factory.h"
#include "backends/fs/romfs/romfs-fs.h"
#include "backends/fs/romfs/romfsstream.h"

#define ROMFS_HEADER_SIZE	16

#define ROMFS_TYPE_DIR		1
#define ROMFS_TYPE_FILE		2

#define ALIGN16(x) (((x) + 15) & ~15)

struct RomfsFileHdr {
	uint32 next;
	uint32 specInfo;
	uint32 size;
	uint32 checksum;
};

struct RomfsFileInfo {
	struct RomfsFileHdr hdr;
	uint8 type;
	Common::String name;
};

// Skips over the romfs header to the first file header
static int32 getFirstFileHeaderOffset(StdioStream *stream) {
	char tmp;

	if (!stream)
		return 0;

	stream->seek(ROMFS_HEADER_SIZE);
	// Read out zero padding
	do {
		stream->read(&tmp, 1);
	} while (tmp);

	return ALIGN16(stream->pos());
}

// Reads the file info at a given offset
static void readFileInfo(StdioStream *stream, struct RomfsFileInfo &info, uint32 offset) {
	char c;

	stream->seek(offset);
	stream->read(&info.hdr, sizeof(info.hdr));

	info.type = FROM_BE_32(info.hdr.next) & 0x7;
	info.hdr.next = FROM_BE_32(info.hdr.next) & 0xfffffff0;
	info.hdr.specInfo = FROM_BE_32(info.hdr.specInfo);
	info.hdr.size = FROM_BE_32(info.hdr.size);

	info.name = "";
	do {
		stream->read(&c, 1);
		if (c)
			info.name += c;
	} while (c);
}

// Composes a list of the path components from a path
static void splitPath(Common::List<Common::String> &list, Common::String &path) {
	const char *first = path.c_str();
	const char *last = first + path.size();
	const char *str;

	for (str = first; str <= last; str++) {
		if (*str == '/' || str == last) {
			list.push_back(Common::String(first, str));
			first = str+1;
		}
	}
}

RomfsFilesystemNode::RomfsFilesystemNode(const Common::String &romFile, const Common::String &pathInRom, const char sep) {
	_sep = sep;
	_romFile = romFile;
	_pathInRom = pathInRom;
	_path = _romFile;
	if (!_pathInRom.empty())
		_path += '/' + _pathInRom;
	_displayName = lastPathComponent(_path, _pathInRom.empty() ? _sep : '/');

	_stream = StdioStream::makeFromPath(_romFile, false);
	validatePathInRom();

	_isValid = _offset != 0;
	_isDirectory = _dirOffset != 0;
}

Common::String RomfsFilesystemNode::getParentPath(const Common::String &path, const char sep) {
	Common::String newPath = normalizePath(path, sep);
	Common::String last = lastPathComponent(newPath, sep);

	newPath.erase(newPath.size() - last.size(), last.size());

	return normalizePath(newPath, sep);
}

void RomfsFilesystemNode::validatePathInRom() {
	struct RomfsFileInfo info;
	Common::List<Common::String> pathComponents;
	uint32 next;

	_dirOffset = 0;
	_dataOffset = 0;
	_dataSize = 0;
	_next = 0;
	_offset = getFirstFileHeaderOffset(_stream);

	if (_pathInRom.empty() || !_offset)
		return;

	splitPath(pathComponents, _pathInRom);
	Common::String curr = pathComponents.front();

	// Walk the path
	while (_offset) {
		readFileInfo(_stream, info, _offset);
		next = info.hdr.next;
		if (info.name == curr) {
			pathComponents.pop_front();
			if (pathComponents.empty())
				break;
			if (info.type == ROMFS_TYPE_DIR)
				next = info.hdr.specInfo;
			curr = pathComponents.front();
		}
		_offset = next;
	}

	// We reached the end of the current dir without a match
	if (!_offset)
		return;

	_next = info.hdr.next;
	if (info.type == ROMFS_TYPE_DIR)
		_dirOffset = info.hdr.specInfo;
	if (info.type == ROMFS_TYPE_FILE) {
		_dataOffset = ALIGN16(_stream->pos());
		_dataSize = info.hdr.size;
	}
}

RomfsFilesystemNode *RomfsFilesystemNode::getChildNode(int32 offset) const {
	struct RomfsFileInfo info;

	if (offset == 0)
		return nullptr;

	readFileInfo(_stream, info, offset);
	if (!_pathInRom.empty())
		info.name = _pathInRom + '/' + info.name;

	return new RomfsFilesystemNode(_romFile, info.name, _sep);
}

bool RomfsFilesystemNode::getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const {
	assert(_isDirectory);

	RomfsFilesystemNode *node = getChildNode(_dirOffset);
	while (node) {
		if (!node->_pathInRom.hasSuffix(".") && !node->_pathInRom.hasSuffix(".."))
			myList.push_back(node);
		node = getChildNode(node->_next);
	}

	return true;
}

AbstractFSNode *RomfsFilesystemNode::getChild(const Common::String &n) const {
	assert(_isDirectory);

	// Make sure the string contains no slashes
	assert(!n.contains('/'));

	Common::String newPath(_path);
	if (_path.lastChar() != '/')
		newPath += '/';
	newPath += n;

	return new RomfsFilesystemNode(_romFile, newPath, _sep);
}

AbstractFSNode *RomfsFilesystemNode::getParent() const {
	Common::String newPath;

	if (_pathInRom.empty()) {
		newPath = getParentPath(_romFile, _sep);
		return g_system->getFilesystemFactory()->makeFileNodePath(newPath);
	}

	newPath = getParentPath(_pathInRom, '/');
	return new RomfsFilesystemNode(_romFile, newPath, _sep);
}

bool RomfsFilesystemNode::exists() const {
	return _isValid;
}

Common::SeekableReadStream *RomfsFilesystemNode::createReadStream() {
	if (_pathInRom.empty())
		return StdioStream::makeFromPath(_romFile, false);

	return new RomfsStream(_stream, _dataOffset, _dataSize);
}

Common::WriteStream *RomfsFilesystemNode::createWriteStream() {
	if (_pathInRom.empty())
		return StdioStream::makeFromPath(_romFile, true);

	return nullptr;
}
