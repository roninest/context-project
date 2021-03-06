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
#include "DesktopIcon.h"

#include <regex>
#include <filesystem>
#include <fstream>

using namespace std;

namespace xio {

DesktopIcon::DesktopIcon()
	: exec_("demo"), version_("1.0"), name_("XioDemo"),
	  skeleton_{"[Desktop Entry]\n"
				"Version=VERSION\n"
				"Name=NAME\n"
				"Comment=COMMENT\n"
				"TryExec=EXEC\n"
				"Exec=EXEC\n"
				"GenericName=NAME\n"
				"Icon=ICON\n"
				"Path=PATH\n"
				"Terminal=false\n"
				"Type=Application\n"} {

}

//----------------------------------------------------------------------------------------------------------------------
DesktopIcon::~DesktopIcon() = default;

//----------------------------------------------------------------------------------------------------------------------
void DesktopIcon::Init() {
  run_dir_ = filesystem::current_path().string();
  output_ = skeleton_;
  output_ = regex_replace(output_, regex("EXEC"), run_dir_ + "/" + exec_);
  output_ = regex_replace(output_, regex("PATH"), run_dir_);
  output_ = regex_replace(output_, regex("ICON"), icon_);
  output_ = regex_replace(output_, regex("NAME"), name_);
  output_ = regex_replace(output_, regex("COMMENT"), name_);
  output_ = regex_replace(output_, regex("VERSION"), version_);
}

//----------------------------------------------------------------------------------------------------------------------
void DesktopIcon::Save(const string &icon_name) {
  string home_dir = string(getenv("HOME"));
  string path = home_dir + "/.local/share/applications/" + icon_name + ".desktop";

  ifstream in;
  in.open(path);
  if (in.is_open()) {
	string s;
	getline(in, s, '\0');
	in.close();

	if (s==output_)
	  return;
  }

  ofstream out;
  out.open(path);
  if (out.is_open()) {
	out << output_;
  }
}

} //namespace
