#pragma once
#include <filesystem>
#include <string>

class AbstractFileManager;

namespace fs = std::filesystem;

void viewTextFile(const fs::path& filepath);
fs::path selectDrive(AbstractFileManager& fm);
