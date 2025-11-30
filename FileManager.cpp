#include "FileManager.h"
#include <regex>
#include <fstream>
#include <iostream>

using namespace std;

string FileManager::maskToRegex(const string& mask) {
    string r = "^";

    for (char c : mask) {
        switch (c) {
        case '*': r += ".*"; break;
        case '?': r += "."; break;
        case '.': r += "\\."; break;
        case '\\': r += "\\\\"; break;
        default:
            if (string("[](){}+^$|").find(c) != string::npos)
                r += '\\';
            r += c;
        }
    }

    r += "$";
    return r;
}

FileManager::FileManager(const fs::path& startPath)
    : currentPath(startPath) {
}

const fs::path& FileManager::getCurrentPath() const {
    return currentPath;
}

void FileManager::setCurrentPath(const fs::path& p) {
    currentPath = p;
}

vector<fs::directory_entry> FileManager::getDirectories() const {
    vector<fs::directory_entry> dirs;
    try {
        for (const auto& entry : fs::directory_iterator(currentPath))
            if (entry.is_directory())
                dirs.push_back(entry);
    }
    catch (...) {}
    return dirs;
}

vector<fs::directory_entry> FileManager::getFiles() const {
    vector<fs::directory_entry> files;
    try {
        for (const auto& entry : fs::directory_iterator(currentPath))
            if (!entry.is_directory())
                files.push_back(entry);
    }
    catch (...) {}
    return files;
}

void FileManager::goToDirectory(const fs::path& dirPath) {
    if (fs::exists(dirPath) && fs::is_directory(dirPath))
        currentPath = dirPath;
}

void FileManager::goUp() {
    if (currentPath.has_parent_path())
        currentPath = currentPath.parent_path();
}

bool FileManager::deleteEntry(const fs::directory_entry& entry) {
    try {
        if (entry.is_directory())
            fs::remove_all(entry);
        else
            fs::remove(entry);
        return true;
    }
    catch (...) {
        return false;
    }
}

bool FileManager::createDirectory(const string& name) {
    try {
        return fs::create_directory(currentPath / name);
    }
    catch (...) {
        return false;
    }
}

bool FileManager::createFile(const string& name) {
    try {
        ofstream f(currentPath / name);
        bool ok = f.is_open();
        f.close();
        return ok;
    }
    catch (...) {
        return false;
    }
}

bool FileManager::renameEntry(const fs::path& oldPath, const string& newName) {
    try {
        fs::rename(oldPath, oldPath.parent_path() / newName);
        return true;
    }
    catch (...) {
        return false;
    }
}

bool FileManager::copyEntry(const fs::path& from, const fs::path& to) {
    try {
        if (fs::is_directory(from)) {
            fs::copy(from, to, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
        }
        else {
            fs::copy_file(from, to, fs::copy_options::overwrite_existing);
        }
        return true;
    }
    catch (...) {
        return false;
    }
}

bool FileManager::moveEntry(const fs::path& from, const fs::path& to) {
    try {
        fs::rename(from, to);
        return true;
    }
    catch (...) {
        return false;
    }
}

uintmax_t FileManager::getSize(const fs::path& p) const {
    try {
        if (fs::is_regular_file(p))
            return fs::file_size(p);

        if (fs::is_directory(p)) {
            uintmax_t total = 0;
            for (auto& entry : fs::recursive_directory_iterator(p))
                if (entry.is_regular_file())
                    total += fs::file_size(entry);
            return total;
        }
    }
    catch (...) {}
    return 0;
}

vector<fs::path> FileManager::searchByMask(const fs::path& start, const string& mask) const {
    vector<fs::path> result;
    string regexStr = maskToRegex(mask);
    regex re(regexStr, regex_constants::icase);

    try {
        for (auto& entry : fs::recursive_directory_iterator(start)) {
            string name = entry.path().filename().string();
            if (regex_match(name, re))
                result.push_back(entry.path());
        }
    }
    catch (...) {}

    return result;
}

vector<fs::path> FileManager::getDrives() const {
    vector<fs::path> drives;

#ifdef _WIN32
    for (char letter = 'C'; letter <= 'Z'; ++letter) {
        string root;
        root.push_back(letter);
        root += ":\\";
        if (fs::exists(root))
            drives.emplace_back(root);
    }
#else
    drives.emplace_back("/");
#endif

    return drives;
}
