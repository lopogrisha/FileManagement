#include <iostream>
#include <vector>
#include <conio.h>
#include <filesystem>
#include "FileManager.h"
#include "Utils.h"

using namespace std;
namespace fs = std::filesystem;

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
                cout << "Search:\n";
                cout << "1 - by file name\n";
                cout << "2 - by extension\n";
                cout << "\nEnter choice: ";

                int choice;
                cin >> choice;

                cin.clear();
                cin.ignore(1000, '\n');

                if (choice == 1) {
                    system("cls");
                    cout << "Enter file name (example: test.txt): ";

                    string name;
                    getline(cin, name);

                    auto results = fm.findByName(name, fm.getCurrentPath());

                    system("cls");
                    cout << "Found " << results.size() << " file(s):\n\n";
                    for (auto& p : results)
                        cout << p.string() << "\n";

                    cout << "\nPress any key...";
                    _getch();
                }
                else if (choice == 2) {
                    system("cls");
                    cout << "Enter extension (example: .txt): ";

                    string ext;
                    getline(cin, ext);

                    auto results = fm.findByExtension(ext, fm.getCurrentPath());

                    system("cls");
                    cout << "Found " << results.size() << " file(s):\n\n";
                    for (auto& p : results)
                        cout << p.string() << "\n";

                    cout << "\nPress any key...";
                    _getch();
                }
                }



        }
        else if (key == 27) {
            cout << "Exiting..." << endl;
            break;
        }
    }
    return 0;
}