#include "krspch.h"
#include "Filesystem.h"

#include <ostream>
#include <filesystem>
#include <string>
#include <regex>
#include <fstream>
#include <shellapi.h>
#include <Shlwapi.h>

namespace fs = std::filesystem;


namespace Kairos {
	namespace Filesystem {
		void CreateTextFile(const std::string& filePath, const std::string& text)
		{
			std::ofstream outfile(filePath);
			if (outfile.fail()) {
				KRS_CORE_ERROR("Issue with creating output file");
				return;
			}
			outfile << text;
			outfile.flush();
			outfile.close();
		}

		std::wstring StringToWString(const std::string& input)
		{
			const auto slength = static_cast<int>(input.length()) + 1;
			const auto len = MultiByteToWideChar(CP_ACP, 0, input.c_str(), slength, nullptr, 0);
			const auto buf = new wchar_t[len];
			MultiByteToWideChar(CP_ACP, 0, input.c_str(), slength, buf, len);
			std::wstring result(buf);
			delete[] buf;
			return result;
		}

		bool PathExists(const std::string& path)
		{
			if (fs::exists(path))
				return true;
			else {
				KRS_CORE_ERROR("File Path does not exist");
				return false;
			}
			return false;
		}

		std::string GetExecutablePath()
		{
			wchar_t buffer[MAX_PATH];
			GetModuleFileName(NULL, buffer, MAX_PATH);
			PathRemoveFileSpec(buffer);
			PathCombine(buffer, buffer, L"..");
			std::wstring s = fs::absolute(buffer);
			return std::string(s.begin(), s.end());
		}

		void OpenDirectoryWindow(const std::string& path)
		{
			ShellExecute(nullptr, nullptr, StringToWString(path).c_str(), nullptr, nullptr, SW_SHOW);
		}
		std::string GetWorkingDirectory()
		{
			return fs::current_path().generic_string();
		}
		std::string GetParentDirectory(const std::string& path)
		{
			return fs::path(path).parent_path().generic_string();
		}
		std::string GetRootDirectory(const std::string& path)
		{
			return fs::path(path).root_directory().generic_string();
		}
		std::string GetRelativePath(const std::string& path)
		{
			if (fs::path(path).is_relative())
				return path;

			// create absolute paths
			const fs::path p = fs::absolute(path);
			const fs::path r = fs::absolute(GetWorkingDirectory());

			return p.lexically_relative(r).generic_string();
		}
	}


}

