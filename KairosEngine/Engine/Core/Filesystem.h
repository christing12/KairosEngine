#pragma once

#include <Core/EngineCore.h>

#include <vector>

namespace Kairos {
	namespace Filesystem {
		void CreateTextFile(const std::string& filePath, const std::string& text);

		std::wstring StringToWString(const std::string& input);
		bool PathExists(const std::string& path);

		std::string GetExecutablePath();

		// PATH STUFF
		void OpenDirectoryWindow(const std::string& path);
		bool CreateDirectory_(const std::string& path);
		bool Delete(const std::string& path);

		bool CopyFileFromTo(const std::string& srcPath, const std::string& destPath);
		std::string FilenameFromPath(const std::string& path);
		std::string GetWorkingDirectory();
		std::string GetParentDirectory(const std::string& path);
		std::string GetRootDirectory(const std::string& path);
		std::string GetRelativePath(const std::string& path);
		std::vector<std::string> GetFilesInDirectory(const std::string& path);
		std::string NativizeFilePath(const std::string& path);

		static const char* EXTENSION_TEXTURE = ".tx";
		static const char* EXTENSION_MODEL = ".model";
	}
}