#include <iostream>
#include <filesystem>
#include <string>
#include <locale>
#include <vector>
#include <conio.h>
#include <fstream>
#include <regex>
#include <limits>

#define _CRT_SECURE_NO_WARNINGS

using namespace std;
namespace fs = std::filesystem;

void viewTextFile(const fs::path& filepath) {
    system("cls");
    ifstream file(filepath);
    if (!file.is_open()) {
        cout << "Failed to open file!" << endl;
        _getch();
        return;
    }

    string line;
    while (getline(file, line)) {
        cout << line << endl;
    }
    file.close();

    cout << "\nPress any key to return...";
    _getch();
}

class AbstractFileManager {
public:
    virtual const fs::path& getCurrentPath() const = 0;
    virtual void setCurrentPath(const fs::path& p) = 0;

    virtual vector<fs::directory_entry> getDirectories() const = 0;
    virtual vector<fs::directory_entry> getFiles() const = 0;

    virtual void goToDirectory(const fs::path& dirPath) = 0;
    virtual void goUp() = 0;

    virtual bool deleteEntry(const fs::directory_entry& entry) = 0;
    virtual bool createDirectory(const string& name) = 0;
    virtual bool createFile(const string& name) = 0;

    virtual bool renameEntry(const fs::path& oldPath, const string& newName) = 0;
    virtual bool copyEntry(const fs::path& from, const fs::path& to) = 0;
    virtual bool moveEntry(const fs::path& from, const fs::path& to) = 0;

    virtual uintmax_t getSize(const fs::path& p) const = 0;
    virtual vector<fs::path> searchByMask(const fs::path& start, const string& mask) const = 0;

    virtual vector<fs::path> getDrives() const = 0;

    virtual ~AbstractFileManager() {}
};

class FileManager : public AbstractFileManager {
private:
    fs::path currentPath;

    static string maskToRegex(const string& mask) {
        string r = "^";

        for (char c : mask) {
            switch (c) {
            case '*': r += ".*"; break;
            case '?': r += "."; break;
            case '.': r += "\\."; break;
            case '\\': r += "\\\\"; break;
            default:
                if (std::string("[](){}+^$|").find(c) != string::npos)
                    r += '\\';
                r += c;
            }
        }

        r += "$";
        return r;
    }


public:
    FileManager(const fs::path& startPath = fs::current_path())
        : currentPath(startPath) {
    }

    const fs::path& getCurrentPath() const override {
        return currentPath;
    }

    void setCurrentPath(const fs::path& p) override {
        currentPath = p;
    }

    vector<fs::directory_entry> getDirectories() const override {
        vector<fs::directory_entry> dirs;
        try {
            for (const auto& entry : fs::directory_iterator(currentPath)) {
                if (entry.is_directory())
                    dirs.push_back(entry);
            }
        }
        catch (...) {}
        return dirs;
    }

    vector<fs::directory_entry> getFiles() const override {
        vector<fs::directory_entry> files;
        try {
            for (const auto& entry : fs::directory_iterator(currentPath)) {
                if (!entry.is_directory())
                    files.push_back(entry);
            }
        }
        catch (...) {}
        return files;
    }

    void goToDirectory(const fs::path& dirPath) override {
        if (fs::exists(dirPath) && fs::is_directory(dirPath)) {
            currentPath = dirPath;
        }
    }

    void goUp() override {
        if (currentPath.has_parent_path()) {
            currentPath = currentPath.parent_path();
        }
    }

    bool deleteEntry(const fs::directory_entry& entry) override {
        try {
            if (entry.is_directory()) {
                fs::remove_all(entry);
            }
            else {
                fs::remove(entry);
            }
            return true;
        }
        catch (...) {
            return false;
        }
    }

    bool createDirectory(const string& name) override {
        try {
            return fs::create_directory(currentPath / name);
        }
        catch (...) {
            return false;
        }
    }

    bool createFile(const string& name) override {
        try {
            fs::path p = currentPath / name;
            ofstream f(p);
            bool ok = f.is_open();
            f.close();
            return ok;
        }
        catch (...) {
            return false;
        }
    }

    bool renameEntry(const fs::path& oldPath, const string& newName) override {
        try {
            fs::path newPath = oldPath.parent_path() / newName;
            fs::rename(oldPath, newPath);
            return true;
        }
        catch (...) {
            return false;
        }
    }

    bool copyEntry(const fs::path& from, const fs::path& to) override {
        try {
            if (fs::is_directory(from)) {
                fs::copy(from, to,
                    fs::copy_options::recursive |
                    fs::copy_options::overwrite_existing);
            }
            else {
                fs::copy_file(from, to,
                    fs::copy_options::overwrite_existing);
            }
            return true;
        }
        catch (...) {
            return false;
        }
    }

    bool moveEntry(const fs::path& from, const fs::path& to) override {
        try {
            fs::rename(from, to);
            return true;
        }
        catch (...) {
            return false;
        }
    }

    uintmax_t getSize(const fs::path& p) const override {
        try {
            if (fs::is_regular_file(p)) {
                return fs::file_size(p);
            }
            else if (fs::is_directory(p)) {
                uintmax_t total = 0;
                for (auto& entry : fs::recursive_directory_iterator(p)) {
                    if (entry.is_regular_file()) {
                        total += fs::file_size(entry);
                    }
                }
                return total;
            }
        }
        catch (...) {}
        return 0;
    }

    vector<fs::path> searchByMask(const fs::path& start, const string& mask) const override {
        vector<fs::path> result;
        string regexStr = maskToRegex(mask);
        std::regex re(regexStr, std::regex_constants::icase);

        try {
            for (auto& entry : fs::recursive_directory_iterator(start)) {
                string name = entry.path().filename().string();
                if (std::regex_match(name, re)) {
                    result.push_back(entry.path());
                }
            }
        }
        catch (...) {}
        return result;
    }

    vector<fs::path> getDrives() const override {
        vector<fs::path> drives;
#ifdef _WIN32
        for (char letter = 'C'; letter <= 'Z'; ++letter) {
            string root;
            root.push_back(letter);
            root += ":\\";
            if (fs::exists(root)) {
                drives.emplace_back(root);
            }
        }
#else
        drives.emplace_back("/");
#endif
        return drives;
    }
};

fs::path selectDrive(AbstractFileManager& fm) {
    auto drives = fm.getDrives();
    if (drives.empty()) {
        return fs::current_path();
    }

    while (true) {
        system("cls");
        cout << "Select drive:\n\n";
        for (size_t i = 0; i < drives.size(); ++i) {
            cout << (i + 1) << ") " << drives[i].string() << "\n";
        }
        cout << "0) Current path (" << fs::current_path().string() << ")\n";
        cout << "\nChoice: ";
        int choice;
        cin >> choice;

        if (choice == 0) return fs::current_path();
        if (choice > 0 && (size_t)choice <= drives.size()) {
            return drives[choice - 1];
        }
    }
}


bool isSubPath(const fs::path& fromDir, const fs::path& destDir) {
    try {
        fs::path from = fs::canonical(fromDir);
        fs::path dest = fs::canonical(destDir);
        auto fromStr = from.string();
        auto destStr = dest.string();
        if (destStr.size() < fromStr.size()) return false;
        if (destStr.compare(0, fromStr.size(), fromStr) != 0) return false;
        return true;
    }
    catch (...) {
        return false;
    }
}

int main() {
    setlocale(LC_ALL, "C");

    FileManager fm;
    fs::path startPath = selectDrive(fm);
    fm.setCurrentPath(startPath);

    int selected = 0;

    while (true) {
        system("cls");

        cout << "Current path: " << fm.getCurrentPath() << endl;
        cout << "------------------------------------------" << endl;

        vector<fs::directory_entry> dirs = fm.getDirectories();
        vector<fs::directory_entry> files = fm.getFiles();

        int index = 0;
        for (const auto& entry : dirs) {
            if (index == selected) cout << ">> ";
            else cout << "   ";
            cout << entry.path().filename().string() << "\t<DIR>" << endl;
            index++;
        }

        for (const auto& entry : files) {
            if (index == selected) cout << ">> ";
            else cout << "   ";
            cout << entry.path().filename().string();
            try {
                cout << "\t" << fs::file_size(entry);
            }
            catch (...) {}
            cout << endl;
            index++;
        }

        cout << "\nControls: ArrowUp/ArrowDown - move, Enter - open, Backspace - up\n";
        cout << "Del - delete, F2 - create file, F7 - create dir\n";
        cout << "F3 - rename, F4 - copy, F5 - move, F6 - size, F9 - search, Esc - exit\n";

        int key = _getch();
        int total = (int)(dirs.size() + files.size());

        if (key == 224) {
            key = _getch();
            if (key == 72 && selected > 0) {
                selected--;
            }
            else if (key == 80 && selected < index - 1) {
                selected++;
            }
            else if (key == 83) {
                if (total == 0) continue;
                bool ok = false;
                if (selected < (int)dirs.size()) {
                    ok = fm.deleteEntry(dirs[selected]);
                }
                else {
                    ok = fm.deleteEntry(files[selected - dirs.size()]);
                }
                if (!ok) {
                    cout << "Failed to delete!" << endl;
                    _getch();
                }
                if (selected > 0) selected--;
            }
        }
        else if (key == 13) {
            if (total == 0) continue;

            if (selected < (int)dirs.size()) {
                fm.goToDirectory(dirs[selected].path());
                selected = 0;
            }
            else {
                fs::path filepath = files[selected - dirs.size()].path();
                string ext = filepath.extension().string();
                if (ext == ".txt" || ext == ".cpp" || ext == ".h" || ext == ".log") {
                    viewTextFile(filepath);
                }
            }
        }
        else if (key == 8) {
            fm.goUp();
            selected = 0;
        }
        else if (key == 0) {
            key = _getch();
            if (key == 60) {         
                system("cls");
                cout << "Enter new file name: ";
                string name;
                cin >> name;
                if (!fm.createFile(name)) {
                    cout << "Failed to create file!" << endl;
                    _getch();
                }
            }
            else if (key == 61) {      
                if (total == 0) continue;
                fs::path target;
                if (selected < (int)dirs.size())
                    target = dirs[selected].path();
                else
                    target = files[selected - dirs.size()].path();

                system("cls");
                cout << "Old name: " << target.filename().string() << endl;
                cout << "Enter new name: ";
                string newName;
                cin >> newName;
                if (!fm.renameEntry(target, newName)) {
                    cout << "Failed to rename!" << endl;
                    _getch();
                }
            }
            else if (key == 62) {      
                if (total == 0) continue;

                fs::path from = (selected < (int)dirs.size())
                    ? dirs[selected].path()
                    : files[selected - dirs.size()].path();

                system("cls");
                cout << "Copy from: " << from << endl;
                cout << "Enter destination directory path: ";

                string destStr;
                getline(cin >> ws, destStr);  

                fs::path destDir(destStr);

                if (!fs::exists(destDir) || !fs::is_directory(destDir)) {
                    cout << "Destination must be an existing directory!" << endl;
                    _getch();
                }
                else {
                    fs::path dest = destDir / from.filename();
                    if (!fm.copyEntry(from, dest)) {
                        cout << "Failed to copy!" << endl;
                        _getch();
                    }
                }
            }
            else if (key == 63) {      
                if (total == 0) continue;

                fs::path from = (selected < (int)dirs.size())
                    ? dirs[selected].path()
                    : files[selected - dirs.size()].path();

                system("cls");
                cout << "Move from: " << from << endl;
                cout << "Enter destination directory path: ";

                string destStr;
                getline(cin >> ws, destStr);  

                fs::path destDir(destStr);

                if (!fs::exists(destDir) || !fs::is_directory(destDir)) {
                    cout << "Destination must be an existing directory!" << endl;
                    _getch();
                }
                else {
                    fs::path dest = destDir / from.filename();
                    if (!fm.moveEntry(from, dest)) {
                        cout << "Failed to move!" << endl;
                        _getch();
                    }
                }
            }

            else if (key == 64) {     
                if (total == 0) continue;
                fs::path target;
                if (selected < (int)dirs.size())
                    target = dirs[selected].path();
                else
                    target = files[selected - dirs.size()].path();

                uintmax_t sz = fm.getSize(target);
                system("cls");
                cout << "Path: " << target << endl;
                cout << "Size: " << sz << " bytes" << endl;
                cout << "\nPress any key...";
                _getch();
            }
            else if (key == 65) {      
                system("cls");
                cout << "Enter name for new directory: ";
                string name;
                cin >> name;
                if (!fm.createDirectory(name)) {
                    cout << "Failed to create directory!" << endl;
                    _getch();
                }
            }
            else if (key == 67) {     
                system("cls");
                cout << "Search in: " << fm.getCurrentPath() << endl;
                cout << "Enter mask (e.g. *.txt): ";

                cin.clear();
                cin.sync();      
                string mask;
                getline(cin, mask);

                auto results = fm.searchByMask(fm.getCurrentPath(), mask);

                system("cls");
                cout << "Found " << results.size() << " item(s):\n\n";
                for (auto& p : results) {
                    cout << p.string() << endl;
                }
                cout << "\nPress any key...";
                _getch();
                }


        }
        else if (key == 27) {
            cout << "Exiting..." << endl;
            break;
        }
    }
    return 0;
}
