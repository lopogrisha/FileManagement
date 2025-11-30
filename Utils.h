#pragma once
#include <filesystem>

void viewTextFile(const std::filesystem::path& filepath);
std::filesystem::path selectDrive(class AbstractFileManager& fm);
