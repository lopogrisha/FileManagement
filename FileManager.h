#pragma once
#include "AbstractFileManager.h"

class FileManager : public AbstractFileManager {
private:
    fs::path currentPath;

public:
    FileManager(const fs::path& startPath = fs::current_path());

    const fs::path& getCurrentPath() const override;
    void setCurrentPath(const fs::path& p) override;

    std::vector<fs::directory_entry> getDirectories() const override;
    std::vector<fs::directory_entry> getFiles() const override;

    void goToDirectory(const fs::path& dirPath) override;
    void goUp() override;

    bool deleteEntry(const fs::directory_entry& entry) override;
    bool createDirectory(const std::string& name) override;
    bool createFile(const std::string& name) override;

    bool renameEntry(const fs::path& oldPath, const std::string& newName) override;
    bool copyEntry(const fs::path& from, const fs::path& to) override;
    bool moveEntry(const fs::path& from, const fs::path& to) override;

    uintmax_t getSize(const fs::path& p) const override;

    std::vector<fs::path> findByName(const std::string& filename,
        const fs::path& directory) const override;
    std::vector<fs::path> findByExtension(const std::string& extension,
        const fs::path& directory) const override;

    std::vector<fs::path> getDrives() const override;
};
