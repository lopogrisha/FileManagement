#include "Utils.h"
#include "AbstractFileManager.h"
#include <iostream>
#include <fstream>
#include <conio.h>

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
    while (getline(file, line))
        cout << line << endl;

    cout << "\nPress any key...";
    _getch();
}

fs::path selectDrive(AbstractFileManager& fm) {
    auto drives = fm.getDrives();
    if (drives.empty())
        return fs::current_path();

    while (true) {
        system("cls");
        cout << "Select drive:\n\n";

        for (size_t i = 0; i < drives.size(); i++)
            cout << i + 1 << ") " << drives[i].string() << "\n";

        cout << "0) Current path (" << fs::current_path().string() << ")\n";
        cout << "\nChoice: ";

        int choice;
        cin >> choice;

        if (choice == 0)
            return fs::current_path();

        if (choice > 0 && (size_t)choice <= drives.size())
            return drives[choice - 1];
    }
}
