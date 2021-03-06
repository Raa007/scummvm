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

#include "ResourceManager.h"

namespace Cryo {

ResourceManager::ResourceManager() {
}

ResourceManager::ResourceManager(const Common::String &datFileName) {
	LoadDatFile(datFileName);
}

ResourceManager::~ResourceManager() {
}

bool ResourceManager::LoadDatFile(const Common::String &datFileName) {
	if (_datFile.isOpen()) {
		_datFile.close();
		_files.clear();
	}

	assert(_datFile.open(datFileName));

	uint16 numFiles = _datFile.readUint16LE();

	for (uint16 i = 0; i < numFiles; i++) {
		DatFileEntry entry;

		_datFile.read(entry._name, sizeof(entry._name));
		entry._size = _datFile.readUint32LE();
		entry._offset = _datFile.readUint32LE();
		entry._flag = _datFile.readByte();

		_files.push_back(entry);
	}

	return true;
}

Common::SeekableReadStream *ResourceManager::GetFile(const Common::String &resName, unsigned int hintIndex) {
	// First, try raw disk file so we can support modding/patching

	if (Common::File::exists(resName)) {
		debug("Loading %s from disk", resName);

		Common::File *resource = new Common::File();
		resource->open(resName);
		return resource;
	}

	// Look inside .dat file

	if (_datFile.isOpen()) {
		for (unsigned int i = hintIndex; i < _files.size(); i++) {
			if (!resName.compareToIgnoreCase(_files[i]._name)) {
				debug("Loading %s from dat file", resName);
				Common::SeekableSubReadStream *resource = new Common::SeekableSubReadStream(&_datFile, _files[i]._offset, _files[i]._offset + _files[i]._size);
				return resource;
			}
		}
	}

	debug("Unable to load %s - does't exists", resName);
	return nullptr;
}

Common::SeekableReadStream *ResourceManager::GetFile(unsigned int resIndex) {
	if (_files.size() > resIndex) {
		return GetFile(Common::String(_files[resIndex]._name), resIndex);
	}

	return nullptr;
}

void *ResourceManager::StreamToBuffer(Common::SeekableReadStream *stream, unsigned int *size) {
	if (!stream)
		return nullptr;

	unsigned int readSize = stream->size();
	byte *data = new byte[readSize + 1];
	readSize = stream->read(data, readSize);

	if (size)
		*size = readSize;
	return data;
}

void *ResourceManager::GetData(const Common::String &resName, unsigned int *size) {
	Common::SeekableReadStream *resource = GetFile(resName);
	void *data = StreamToBuffer(resource, size);
	delete resource;
	return data;
}

void *ResourceManager::GetData(int resIndex, unsigned int *size) {
	Common::SeekableReadStream *resource = GetFile(resIndex);
	void *data = StreamToBuffer(resource, size);
	delete resource;
	return data;
}
}
