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
#include <ppltasks.h>
#include "backends/fs/wp81/wp81-fs.h"
#include "backends/fs/zip/zip-fs.h"
#include "backends/fs/stdiostream.h"

using namespace concurrency;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Core;

WP81FilesystemNode::WP81FilesystemNode() {
	_path = "";
	_isValid = false;
	_isReadonly = true;
	_isDirectory = true;
	_isPseudoRoot = true;
}

WP81FilesystemNode::WP81FilesystemNode(const Common::String &path) {
	_path = path;
	setFlags();
}

WP81FilesystemNode::WP81FilesystemNode(const Common::String &path, const Common::String &displayName) {
	_displayName = displayName;
	_isDirectory = true;
	_isValid = true;
	_isPseudoRoot = false;
	_path = path;
}

char* WP81FilesystemNode::toAscii(const wchar_t *str) {
	static char asciiString[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, str, wcslen(str) + 1, asciiString, sizeof(asciiString), NULL, NULL);
	return asciiString;
}

const wchar_t* WP81FilesystemNode::toUnicode(const char *str) {
	static wchar_t unicodeString[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, str, strlen(str) + 1, unicodeString, sizeof(unicodeString) / sizeof(wchar_t));
	return unicodeString;
}

void WP81FilesystemNode::createDir(StorageFolder^ dir, Platform::String ^subDir ) {
	create_task(dir->CreateFolderAsync(subDir)).then([](task<StorageFolder^> t){
		try{ t.get(); }	catch (...) {}
	});
}

void WP81FilesystemNode::setFlags() {
	_isValid = false;
	_isDirectory = false;
	_isReadonly = false;
	_isPseudoRoot = false;

	if (_path.empty())
		return;

	bool done = false;
	const char *start = _path.c_str();
	const char *end = lastPathComponent(_path, '\\');
	Common::String p = Common::String(start, end - start);

	Platform::String ^parent = ref new Platform::String(toUnicode(p.c_str()));
	Platform::String ^child = ref new Platform::String(toUnicode(end));

	auto t = create_task(StorageFolder::GetFolderFromPathAsync(parent));
	t.then([this, &done, child](StorageFolder ^folder) -> task<IStorageItem ^> {
		return create_task(folder->GetItemAsync(child));
	}).then([this, &done](task<IStorageItem ^> t) {
		try {
			IStorageItem ^item = t.get();
			_isValid = true;
			_isDirectory = (item->Attributes & FileAttributes::Directory) == FileAttributes::Directory;
			_isReadonly = (item->Attributes & FileAttributes::ReadOnly) == FileAttributes::ReadOnly;
			_displayName = toAscii(item->Name->Data());
			_isPseudoRoot = false;
		}
		catch (...) {
		}
		done = true;
	});

	while (!done)
		CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneIfPresent);
}

bool WP81FilesystemNode::isReadable() const {
	return _isValid;
}

bool WP81FilesystemNode::isWritable() const {
	return !_isReadonly;
}

bool WP81FilesystemNode::exists() const {
	return _isValid;
}

AbstractFSNode *WP81FilesystemNode::getChild(const Common::String &n) const {
	assert(_isDirectory);

	// Make sure the string contains no slashes
	assert(!n.contains('/'));

	Common::String newPath(_path);
	if (_path.lastChar() != '\\')
		newPath += '\\';
	newPath += n;

	return new WP81FilesystemNode(newPath);
}

bool WP81FilesystemNode::getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const {
	bool done = false;

	assert(_isDirectory);

	if (_isPseudoRoot || _path.empty()) {
		myList.push_back(new WP81FilesystemNode(toAscii(ApplicationData::Current->LocalFolder->Path->Data()), "LocalFolder"));
		myList.push_back(new WP81FilesystemNode(toAscii(Windows::ApplicationModel::Package::Current->InstalledLocation->Path->Data()), "InstalledLocation"));

		// Get SD card (if exists)
		StorageFolder ^removableDevices = KnownFolders::RemovableDevices;
		auto t = create_task(removableDevices->GetFoldersAsync());
		t.then([&myList, &done](IVectorView<StorageFolder ^> ^folders) {
			try {
				StorageFolder ^sdCard = folders->GetAt(0);
				WP81FilesystemNode node;

				node._path = toAscii(sdCard->Path->Data());
				node._displayName = "SDCard";
				node._isValid = true;
				node._isDirectory = true;
				node._isReadonly = true;
				node._isPseudoRoot = false;

				myList.push_back(new WP81FilesystemNode(node));

			} catch (...) {
			}
			done = true;
		});

		while (!done)
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneIfPresent);

		return true;
	}

	Platform::String ^path = ref new Platform::String(toUnicode(_path.c_str()));

	auto t = create_task(StorageFolder::GetFolderFromPathAsync(path));
	t.then([&myList, &done](StorageFolder ^folder) -> task<IVectorView<IStorageItem ^> ^> {
		return create_task(folder->GetItemsAsync());
	}).then([&myList, &done](task<IVectorView<IStorageItem ^> ^> t) {
		try {
			IVectorView<IStorageItem ^> ^items = t.get();
			for (auto it = items->First(); it->HasCurrent; it->MoveNext()) {
				IStorageItem ^item = it->Current;
				WP81FilesystemNode node;

				node._path = toAscii(item->Path->Data());
				node._displayName = toAscii(item->Name->Data());
				node._isValid = true;
				node._isDirectory = (item->Attributes & FileAttributes::Directory) == FileAttributes::Directory;
				node._isReadonly = (item->Attributes & FileAttributes::ReadOnly) == FileAttributes::ReadOnly;
				node._isPseudoRoot = false;

				if (node._displayName.hasSuffix(".scummz")) {
					myList.push_back(new ZipFilesystemNode(node._path, "", '\\'));
					continue;
				}

				myList.push_back(new WP81FilesystemNode(node));
			}
		} catch (...) {
		}
		done = true;
	});

	while (!done)
		CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneIfPresent);

	return true;
}

AbstractFSNode *WP81FilesystemNode::getParent() const {
	if (_isPseudoRoot)
		return 0;

	// FIXME: this doesn't return to the pseudo root at the right point(s)
	WP81FilesystemNode *node = new WP81FilesystemNode();
	if (_path.empty())
		return node;

	const char *start = _path.c_str();
	const char *end = lastPathComponent(_path, '\\');
	node->_path = Common::String(start, end - start);
	node->_displayName = lastPathComponent(node->_path, '\\');
	node->_isValid = true;
	node->_isDirectory = true;
	node->_isPseudoRoot = false;

	return node;
}

Common::SeekableReadStream *WP81FilesystemNode::createReadStream() {
	return StdioStream::makeFromPath(getPath(), false);
}

Common::WriteStream *WP81FilesystemNode::createWriteStream() {
	return StdioStream::makeFromPath(getPath(), true);
}
