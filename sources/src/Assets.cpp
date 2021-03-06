//MIT License
//
//Copyright (c) 2021 Andrei Vasilev
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include "pcheader.h"
#include <iostream>
#include <filesystem>
#include "Assets.h"
#include "Exception.h"

using namespace std;
namespace fs = filesystem;

namespace xio {

static std::string FindPath(const std::string& file, int depth = 3) {
	string path = file;

	for (int i = 0; i < depth; i++) {
		if (fs::exists(path))
			return path;
		else
			path = string("../") + path;
	}

	return string();
}

//----------------------------------------------------------------------------------------------------------------------
bool Assets::StringSanityCheck(const string &str) {
  if (str[0]=='#') {
	return true;
  }

  for (auto it : str) {
	if (isalpha(it) || isdigit(it) || it=='.' || it==',' || it==';' || it=='_' || it=='-' || it=='/') {
	} else {
	  return false;
	}
  }

  return true;
}

//----------------------------------------------------------------------------------------------------------------------
void Assets::LeftTrim(string &s) {
  auto it = find_if(s.begin(), s.end(), [](char c) { return !isspace < char > (c, locale::classic()); });
  s.erase(s.begin(), it);
}

//----------------------------------------------------------------------------------------------------------------------
void Assets::RightTrim(string &s) {
  auto it = find_if(s.rbegin(), s.rend(), [](char c) { return !isspace < char > (c, locale::classic()); });
  s.erase(it.base(), s.end());
}

//----------------------------------------------------------------------------------------------------------------------
void Assets::TrimString(string &s) {
  RightTrim(s);
  LeftTrim(s);
}

//----------------------------------------------------------------------------------------------------------------------
void Assets::PrintPathList(const vector <tuple<string, string, string>> &path_list) {
  cout << "Path list:\n";

  for (const auto &it : path_list) {
	cout << "Path: " << get<0>(it) << "; Type: " << get<1>(it) << "; Group: " << get<2>(it) << ";\n";
  }

  cout << '\n';
}

//----------------------------------------------------------------------------------------------------------------------
void Assets::PrintStringList(const vector <string> &string_list) {
  cout << "Path list:\n";

  for (const auto &it : string_list) {
	cout << "String : " << it << ";\n";
  }

  cout << '\n';
}

//----------------------------------------------------------------------------------------------------------------------
void Assets::LoadResources() {
  Ogre::ResourceGroupManager::getSingletonPtr()->initialiseAllResourceGroups();
}

//----------------------------------------------------------------------------------------------------------------------
void
Assets::InitGeneralResources(const vector <string> &path_list,
							 const string &resource_file,
							 bool verbose) {
  const string default_group_name = Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME;

  vector <string> file_list;
  vector <string> dir_list;
  vector <tuple<string, string, string>> resource_list;
  auto &ogre_resource_manager = Ogre::ResourceGroupManager::getSingleton();

  for (const auto &it : path_list) {
	string path = FindPath(it);

	if (fs::exists(path))
	  resource_list.push_back({path, "FileSystem", default_group_name});
  }

  if (!resource_file.empty()) {
	fstream list_file;
	list_file.open(resource_file);

	string line;
	string path;
	string type;
	string group;

	if (list_file.is_open()) {
	  while (getline(list_file, line)) {
		TrimString(line);

		if (!StringSanityCheck(line)) {
		  throw Exception(string("Sanity check of file ") + resource_file + " is failed. Aborting.");
		}

		if (line[0]=='#') {
		  continue;
		}

		stringstream ss(line);
		getline(ss, path, ',');
		getline(ss, type, ',');
		getline(ss, group, ';');
		resource_list.push_back({path, type, group});
	  }

	  list_file.close();
	}
  }

  if (verbose)
	PrintPathList(resource_list);

  const vector <string> extensions_list =
	  {".glsl", ".glslt", ".hlsl", ".hlslt", ".gles", ".cg", ".vert", ".frag", ".material", ".compositor",
	   ".particle",
	   ".fx", ".program", ".dds", ".bmp", ".png", ".tga", ".jpg",
	   ".jpeg", ".mesh", ".xml", ".scene", ".json", ".wav", ".ogg", ".mp3", ".flac"};

  for (const auto &it : resource_list) {
	ogre_resource_manager.addResourceLocation(get<0>(it), get<1>(it), get<2>(it));
	if (find(begin(path_list), end(path_list), get<0>(it))==end(path_list)) {
	  dir_list.push_back(get<0>(it));
	} else {
//	  throw Exception("Path " + get<0>(it) + " already registered. Aborting.");
	}

	for (auto jt = fs::recursive_directory_iterator(get<0>(it)); jt!=fs::recursive_directory_iterator(); ++jt) {
	  const auto file_path = jt->path().string();
	  const auto file_name = jt->path().filename().string();

	  if (jt->is_directory()) {
		if (verbose) {
		  cout << "Parsing directory:  " << file_path << '\n';
		}
		if (find(begin(path_list), end(path_list), file_name)==end(path_list)) {
		  dir_list.push_back(file_name);
		} else {
//		  throw Exception("Path " + file_name + " already registered. Aborting.");
		}

		ogre_resource_manager.addResourceLocation(file_path, "FileSystem", get<2>(it));

	  } else if (jt->is_regular_file()) {
		if (verbose) {
		  cout << "Parsing file:  " << file_path << '\n';
		}
		if (fs::path(file_path).extension()==".zip") {
		  if (find(begin(file_list), end(file_list), file_name)==end(file_list)) {
			if (find(begin(extensions_list), end(extensions_list), fs::path(file_name).extension())!=end(extensions_list)) {
			  file_list.push_back(file_name);
			}
		  } else {
//			throw Exception("File " + file_name + " already exists. Aborting.");
		  }

		  ogre_resource_manager.addResourceLocation(file_path, "Zip", get<2>(it));
		}
	  }
	}
  }
}

} //namespace
