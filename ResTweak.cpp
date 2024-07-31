#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>

#define CFG_FILE "config.cfg"

// Function to get the highest available refresh rate for a given resolution
int GetHighestRefreshRate(int width, int height) {
    DEVMODE devMode;
    ZeroMemory(&devMode, sizeof(devMode));
    devMode.dmSize = sizeof(devMode);
    devMode.dmPelsWidth = width;
    devMode.dmPelsHeight = height;
    devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

    int highestRefreshRate = 0;
    for (int i = 0; EnumDisplaySettings(NULL, i, &devMode); i++) {
        if (devMode.dmPelsWidth == width && devMode.dmPelsHeight == height) {
            if (devMode.dmDisplayFrequency > highestRefreshRate) {
                highestRefreshRate = devMode.dmDisplayFrequency;
            }
        }
    }

    return highestRefreshRate;
}

// Function to change the display resolution and refresh rate
bool ChangeResolutionAndRefreshRate(int width, int height, int refreshRate) {
    DEVMODE devMode;
    ZeroMemory(&devMode, sizeof(devMode));
    devMode.dmSize = sizeof(devMode);

    if (!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode)) {
        std::cerr << "Failed to get current display settings.\n";
        return false;
    }

    devMode.dmPelsWidth = width;
    devMode.dmPelsHeight = height;
    devMode.dmDisplayFrequency = refreshRate;
    devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;

    LONG result = ChangeDisplaySettingsEx(NULL, &devMode, NULL, CDS_UPDATEREGISTRY, NULL);
    if (result != DISP_CHANGE_SUCCESSFUL) {
        std::cerr << "Failed to change resolution and refresh rate. Error code: " << result << std::endl;
        return false;
    }

    return true;
}

// Function to get the current screen resolution and refresh rate
bool GetCurrentResolutionAndRefreshRate(int& width, int& height, int& refreshRate) {
    DEVMODE devMode;
    ZeroMemory(&devMode, sizeof(devMode));
    devMode.dmSize = sizeof(devMode);

    if (!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode)) {
        std::cerr << "Failed to get current display settings.\n";
        return false;
    }

    width = devMode.dmPelsWidth;
    height = devMode.dmPelsHeight;
    refreshRate = devMode.dmDisplayFrequency;
    return true;
}

// Function to read resolution values from the configuration file
bool ReadConfig(int& customWidth, int& customHeight, int& nativeWidth, int& nativeHeight, int& nativeRefreshRate) {
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
        } else if (line.find("NR=") == 0) {
            nativeRefreshRate = std::stoi(line.substr(3));
        }
    }

    cfgFile.close();
    return true;
}

// Function to write resolution values to the configuration file
bool WriteConfig(int customWidth, int customHeight, int nativeWidth, int nativeHeight, int nativeRefreshRate) {
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
    outFile << "NR=" << nativeRefreshRate << "\n";

    outFile.close();
    return true;
}

// Main function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    int customWidth = 0, customHeight = 0;
    int nativeWidth = 0, nativeHeight = 0;
    int nativeRefreshRate = 0;

    // Read resolution values from config file
    std::cout << "Attempting to read config file...\n";
    if (!ReadConfig(customWidth, customHeight, nativeWidth, nativeHeight, nativeRefreshRate)) {
        MessageBox(NULL, "Failed to read configuration file", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Get the current screen resolution and refresh rate
    int currentWidth, currentHeight, currentRefreshRate;
    if (!GetCurrentResolutionAndRefreshRate(currentWidth, currentHeight, currentRefreshRate)) {
        MessageBox(NULL, "Failed to get current screen resolution and refresh rate", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Debug output for resolution values
    std::cout << "Custom Resolution: " << customWidth << "x" << customHeight << "\n";
    std::cout << "Native Resolution: " << nativeWidth << "x" << nativeHeight << " @ " << nativeRefreshRate << "Hz\n";
    std::cout << "Current Resolution: " << currentWidth << "x" << currentHeight << " @ " << currentRefreshRate << "Hz\n";

    // Check if the current resolution matches the custom resolution
    if (currentWidth == customWidth && currentHeight == customHeight) {
        std::cout << "Reverting to native resolution...\n";
        if (!ChangeResolutionAndRefreshRate(nativeWidth, nativeHeight, nativeRefreshRate)) {
            MessageBox(NULL, "Failed to revert resolution and refresh rate to native settings", "Error", MB_OK | MB_ICONERROR);
            return 1;
        }
    } else {
        std::cout << "Changing to custom resolution...\n";
        int highestRefreshRate = GetHighestRefreshRate(customWidth, customHeight);
        if (highestRefreshRate == 0) {
            MessageBox(NULL, "Failed to find a suitable refresh rate for custom resolution", "Error", MB_OK | MB_ICONERROR);
            return 1;
        }
        if (!ChangeResolutionAndRefreshRate(customWidth, customHeight, highestRefreshRate)) {
            MessageBox(NULL, "Failed to change resolution and refresh rate to custom settings", "Error", MB_OK | MB_ICONERROR);
            return 1;
        }

        // Save the current resolution to the config file
        std::cout << "Saving current resolution to config file...\n";
        if (!WriteConfig(customWidth, customHeight, nativeWidth, nativeHeight, nativeRefreshRate)) {
            MessageBox(NULL, "Failed to save resolution to config file", "Error", MB_OK | MB_ICONERROR);
            return 1;
        }
    }

    std::cout << "Operations completed successfully.\n";
    return 0;
}
