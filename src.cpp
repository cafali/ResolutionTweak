// github.com/cafali/ResolutionTweak

#include <windows.h>
#include <fstream>
#include <string>

#define CFG_FILE "config.cfg"

// get highest available refresh rate for a given resolution
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

// change the display resolution and refresh rate
bool ChangeResolutionAndRefreshRate(int width, int height, int refreshRate) {
    DEVMODE devMode;
    ZeroMemory(&devMode, sizeof(devMode));
    devMode.dmSize = sizeof(devMode);

    if (!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode)) {
        MessageBox(NULL, "Failed to get current display settings.", "Error", MB_OK | MB_ICONERROR);
        return false;
    }

    devMode.dmPelsWidth = width;
    devMode.dmPelsHeight = height;
    devMode.dmDisplayFrequency = refreshRate;
    devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;

    LONG result = ChangeDisplaySettingsEx(NULL, &devMode, NULL, CDS_UPDATEREGISTRY, NULL);
    if (result != DISP_CHANGE_SUCCESSFUL) {
        MessageBox(NULL, "Failed to change resolution and refresh rate.", "Error", MB_OK | MB_ICONERROR);
        return false;
    }

    return true;
}

// get the current screen resolution and refresh rate
bool GetCurrentResolutionAndRefreshRate(int& width, int& height, int& refreshRate) {
    DEVMODE devMode;
    ZeroMemory(&devMode, sizeof(devMode));
    devMode.dmSize = sizeof(devMode);

    if (!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode)) {
        MessageBox(NULL, "Failed to get current display settings.", "Error", MB_OK | MB_ICONERROR);
        return false;
    }

    width = devMode.dmPelsWidth;
    height = devMode.dmPelsHeight;
    refreshRate = devMode.dmDisplayFrequency;
    return true;
}

// read resolution values from the configuration file
bool ReadConfig(int& customWidth, int& customHeight, int& nativeWidth, int& nativeHeight, int& nativeRefreshRate, int& customRefreshRate) {
    std::ifstream cfgFile(CFG_FILE);
    if (!cfgFile.is_open()) {
        MessageBox(NULL, "Failed to open config file for reading.", "Error", MB_OK | MB_ICONERROR);
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
        } else if (line.find("CR=") == 0) { // Check for custom refresh rate
            customRefreshRate = std::stoi(line.substr(3));
        }
    }

    cfgFile.close();
    return true;
}

// Main
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    int customWidth = 0, customHeight = 0;
    int nativeWidth = 0, nativeHeight = 0;
    int nativeRefreshRate = 0;
    int customRefreshRate = 0; // Added custom refresh rate

    // read resolution from config.cfg
    if (!ReadConfig(customWidth, customHeight, nativeWidth, nativeHeight, nativeRefreshRate, customRefreshRate)) {
        MessageBox(NULL, "Failed to read configuration file", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // get current screen resolution and refresh rate
    int currentWidth, currentHeight, currentRefreshRate;
    if (!GetCurrentResolutionAndRefreshRate(currentWidth, currentHeight, currentRefreshRate)) {
        MessageBox(NULL, "Failed to get current screen resolution and refresh rate", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // check current resolution matches the custom resolution
    if (currentWidth == customWidth && currentHeight == customHeight) {
        if (!ChangeResolutionAndRefreshRate(nativeWidth, nativeHeight, nativeRefreshRate)) {
            MessageBox(NULL, "Failed to revert resolution and refresh rate to native settings", "Error", MB_OK | MB_ICONERROR);
            return 1;
        }
    } else {
        // detect refresh rate to use
        int refreshRateToUse = (customRefreshRate > 0) ? customRefreshRate : GetHighestRefreshRate(customWidth, customHeight);
        if (refreshRateToUse == 0) {
            MessageBox(NULL, "Failed to find a suitable refresh rate for custom resolution", "Error", MB_OK | MB_ICONERROR);
            return 1;
        }
        if (!ChangeResolutionAndRefreshRate(customWidth, customHeight, refreshRateToUse)) {
            MessageBox(NULL, "Failed to change resolution and refresh rate to custom settings", "Error", MB_OK | MB_ICONERROR);
            return 1;
        }

    }

    return 0;
}
