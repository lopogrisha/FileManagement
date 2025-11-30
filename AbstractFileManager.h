#pragma once
#include <filesystem>
#include <vector>
#include <string>

namespace fs = std::filesystem;

class AbstractFileManager {
public:
    virtual const fs::path& getCurrentPath() const = 0;
    virtual void setCurrentPath(const fs::path& p) = 0;

    virtual std::vector<fs::directory_entry> getDirectories() const = 0;
    virtual std::vector<fs::directory_entry> getFiles() const = 0;

    virtual void goToDirectory(const fs::path& dirPath) = 0;
    virtual void goUp() = 0;

    virtual bool deleteEntry(const fs::directory_entry& entry) = 0;
    virtual bool createDirectory(const std::string& name) = 0;
    virtual bool createFile(const std::string& name) = 0;

    virtual bool renameEntry(const fs::path& oldPath, const std::string& newName) = 0;
    virtual bool copyEntry(const fs::path& from, const fs::path& to) = 0;
    virtual bool moveEntry(const fs::path& from, const fs::path& to) = 0;

    virtual uintmax_t getSize(const fs::path& p) const = 0;

    virtual std::vector<fs::path> findByName(const std::string& filename,
        const fs::path& directory) const = 0;
    virtual std::vector<fs::path> findByExtension(const std::string& extension,
        const fs::path& directory) const = 0;

    virtual std::vector<fs::path> getDrives() const = 0;

    virtual ~AbstractFileManager() {}
};
