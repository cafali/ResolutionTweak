#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>

#define CFG_FILE "config.cfg"

// Function to change the display resolution
bool ChangeResolution(int width, int height) {
    DEVMODE devMode;
    ZeroMemory(&devMode, sizeof(devMode));
    devMode.dmSize = sizeof(devMode);

    if (!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode)) {
        std::cerr << "Failed to get current display settings.\n";
        return false;
    }

    devMode.dmPelsWidth = width;
    devMode.dmPelsHeight = height;
    devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

    LONG result = ChangeDisplaySettingsEx(NULL, &devMode, NULL, CDS_UPDATEREGISTRY, NULL);
    if (result != DISP_CHANGE_SUCCESSFUL) {
        std::cerr << "Failed to change resolution. Error code: " << result << std::endl;
        return false;
    }

    return true;
}

// Function to get the current screen resolution
bool GetCurrentResolution(int& width, int& height) {
    DEVMODE devMode;
    ZeroMemory(&devMode, sizeof(devMode));
    devMode.dmSize = sizeof(devMode);

    if (!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode)) {
        std::cerr << "Failed to get current display settings.\n";
        return false;
    }

    width = devMode.dmPelsWidth;
    height = devMode.dmPelsHeight;
    return true;
}

// Function to read resolution values from the configuration file
bool ReadConfig(int& customWidth, int& customHeight, int& nativeWidth, int& nativeHeight) {
    std::ifstream cfgFile(CFG_FILE);
    if (!cfgFile.is_open()) {
        std::cerr << "Failed to open config file for reading.\n";
        return false;
    }

    std::string line;
    while (std::getline(cfgFile, line)) {
        if (line.find("CX=") == 0) {
            customWidth = std::stoi(line.substr(3));
        } else if (line.find("CY=") == 0) {
            customHeight = std::stoi(line.substr(3));
        } else if (line.find("NX=") == 0) {
            nativeWidth = std::stoi(line.substr(3));
        } else if (line.find("NY=") == 0) {
            nativeHeight = std::stoi(line.substr(3));
        }
    }

    cfgFile.close();
    return true;
}

// Function to write resolution values to the configuration file
bool WriteConfig(int customWidth, int customHeight, int nativeWidth, int nativeHeight) {
    std::ofstream outFile(CFG_FILE);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open config file for writing.\n";
        return false;
    }

    outFile << "## CustomRes\n";
    outFile << "CX=" << customWidth << "\n";
    outFile << "CY=" << customHeight << "\n";
    outFile << "\n## NativeRes\n";
    outFile << "NX=" << nativeWidth << "\n";
    outFile << "NY=" << nativeHeight << "\n";

    outFile.close();
    return true;
}

// Main function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    int customWidth = 0, customHeight = 0;
    int nativeWidth = 0, nativeHeight = 0;

    // Read resolution values from config file
    std::cout << "Attempting to read config file...\n";
    if (!ReadConfig(customWidth, customHeight, nativeWidth, nativeHeight)) {
        MessageBox(NULL, "Failed to read configuration file", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Get the current screen resolution
    int currentWidth, currentHeight;
    if (!GetCurrentResolution(currentWidth, currentHeight)) {
        MessageBox(NULL, "Failed to get current screen resolution", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Debug output for resolution values
    std::cout << "Custom Resolution: " << customWidth << "x" << customHeight << "\n";
    std::cout << "Native Resolution: " << nativeWidth << "x" << nativeHeight << "\n";
    std::cout << "Current Resolution: " << currentWidth << "x" << currentHeight << "\n";

    // Check if the current resolution matches the custom resolution
    if (currentWidth == customWidth && currentHeight == customHeight) {
        std::cout << "Reverting to native resolution...\n";
        if (!ChangeResolution(nativeWidth, nativeHeight)) {
            MessageBox(NULL, "Failed to revert resolution to native settings", "Error", MB_OK | MB_ICONERROR);
            return 1;
        }
    } else {
        std::cout << "Changing to custom resolution...\n";
        if (!ChangeResolution(customWidth, customHeight)) {
            MessageBox(NULL, "Failed to change resolution to custom settings", "Error", MB_OK | MB_ICONERROR);
            return 1;
        }

        // Save the current resolution to the config file
        std::cout << "Saving current resolution to config file...\n";
        if (!WriteConfig(customWidth, customHeight, nativeWidth, nativeHeight)) {
            MessageBox(NULL, "Failed to save resolution to config file", "Error", MB_OK | MB_ICONERROR);
            return 1;
        }
    }

    std::cout << "Operations completed successfully.\n";
    return 0;
}
