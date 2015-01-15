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

#include "backends/fs/romfs/romfsstream.h"

RomfsStream::RomfsStream(StdioStream *stream, uint32 dataOffset, uint32 dataSize) :
	_stream(stream), _offset(dataOffset), _size(dataSize) {
	_stream->seek(_offset);
}

bool RomfsStream::err() const {
	return _stream->err();
}

void RomfsStream::clearErr() {
	_stream->clearErr();
}

bool RomfsStream::eos() const {
	return _stream->eos() || (uint32)_stream->pos() >= (_offset + _size);
}

int32 RomfsStream::pos() const {
	return _stream->pos() - _offset;
}

int32 RomfsStream::size() const {
	return _size;
}

bool RomfsStream::seek(int32 offs, int whence) {
	switch (whence) {
	case SEEK_END:
		offs -= _offset + _size;
		break;
	case SEEK_SET:
		offs += _offset;
#if 0
		printf("%s: SEEK_SET %d, %d, %d, %d\n",
		       __FUNCTION__, offs, _offset, _size, _stream->size());
#endif
		break;
	case SEEK_CUR:
		break;
	}
	return _stream->seek(offs, whence);
}

uint32 RomfsStream::read(void *ptr, uint32 len) {
	uint32 streamPos = _stream->pos();
	uint32 end = _offset + _size;

	if (len > end - streamPos )
		len = end - streamPos;

	return _stream->read(ptr, len);
}

bool RomfsStream::flush() {
	return _stream->flush();
}
