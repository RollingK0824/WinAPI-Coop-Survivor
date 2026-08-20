#include "Engine/Core/pch.h"
#include "MainMenuBarPanel.h"
#include "Engine/Core/EngineKernel.h"
#include "Engine/Manager/SceneManager.h"
#include <cstdio>
#include <vector>
#include <sstream>
#include <fstream>

static std::wstring FindMSBuildPath(const std::string& userCustomPath)
{
    // 0. User-specified custom path (if set and exists)
    if (!userCustomPath.empty())
    {
        std::wstring wCustom(userCustomPath.begin(), userCustomPath.end());
        DWORD attribs = GetFileAttributesW(wCustom.c_str());
        if (attribs != INVALID_FILE_ATTRIBUTES && !(attribs & FILE_ATTRIBUTE_DIRECTORY))
        {
            return L"\"" + wCustom + L"\"";
        }
    }

    // 1. Environment Variable MSBUILD_PATH
    wchar_t envPath[MAX_PATH] = { 0 };
    if (GetEnvironmentVariableW(L"MSBUILD_PATH", envPath, MAX_PATH) > 0)
    {
        DWORD attribs = GetFileAttributesW(envPath);
        if (attribs != INVALID_FILE_ATTRIBUTES && !(attribs & FILE_ATTRIBUTE_DIRECTORY))
        {
            return L"\"" + std::wstring(envPath) + L"\"";
        }
    }

    // 2. Environment Variable VSINSTALLDIR (Set by Visual Studio when running inside VS)
    wchar_t vsInstallDir[MAX_PATH] = { 0 };
    if (GetEnvironmentVariableW(L"VSINSTALLDIR", vsInstallDir, MAX_PATH) > 0)
    {
        std::wstring vsDir = vsInstallDir;
        if (!vsDir.empty() && vsDir.back() != L'\\' && vsDir.back() != L'/')
        {
            vsDir += L"\\";
        }
        std::vector<std::wstring> vsCandidates = {
            vsDir + L"MSBuild\\Current\\Bin\\MSBuild.exe",
            vsDir + L"MSBuild\\Current\\Bin\\amd64\\MSBuild.exe",
            vsDir + L"MSBuild\\15.0\\Bin\\MSBuild.exe",
            vsDir + L"MSBuild\\15.0\\Bin\\amd64\\MSBuild.exe"
        };
        for (const auto& candidate : vsCandidates)
        {
            DWORD attribs = GetFileAttributesW(candidate.c_str());
            if (attribs != INVALID_FILE_ATTRIBUTES && !(attribs & FILE_ATTRIBUTE_DIRECTORY))
            {
                return L"\"" + candidate + L"\"";
            }
        }
    }

    // Enumerate system drives (C:, D:, E:, F:, etc.)
    std::vector<std::wstring> drivePrefixes;
    DWORD drivesMask = GetLogicalDrives();
    for (int i = 0; i < 26; ++i)
    {
        if (drivesMask & (1 << i))
        {
            wchar_t driveLetter = L'A' + i;
            std::wstring d;
            d += driveLetter;
            d += L":\\";
            drivePrefixes.push_back(d);
        }
    }

    // 3. Search vswhere.exe across all system drives
    std::wstring foundVswhere = L"";
    for (const auto& drive : drivePrefixes)
    {
        std::vector<std::wstring> vswhereCandidates = {
            drive + L"Program Files (x86)\\Microsoft Visual Studio\\Installer\\vswhere.exe",
            drive + L"Program Files\\Microsoft Visual Studio\\Installer\\vswhere.exe",
            drive + L"Microsoft Visual Studio\\Installer\\vswhere.exe",
            drive + L"Visual Studio\\Installer\\vswhere.exe"
        };
        for (const auto& cand : vswhereCandidates)
        {
            DWORD attribs = GetFileAttributesW(cand.c_str());
            if (attribs != INVALID_FILE_ATTRIBUTES && !(attribs & FILE_ATTRIBUTE_DIRECTORY))
            {
                foundVswhere = cand;
                break;
            }
        }
        if (!foundVswhere.empty()) break;
    }

    if (!foundVswhere.empty())
    {
        SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
        HANDLE hReadPipe = NULL, hWritePipe = NULL;
        if (CreatePipe(&hReadPipe, &hWritePipe, &saAttr, 0))
        {
            SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

            STARTUPINFOW si = { sizeof(si) };
            si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
            si.hStdOutput = hWritePipe;
            si.hStdError = hWritePipe;
            si.wShowWindow = SW_HIDE;
            PROCESS_INFORMATION pi = { 0 };

            std::wstring cmd = L"\"" + foundVswhere + L"\" -latest -requires Microsoft.Component.MSBuild -find MSBuild\\**\\Bin\\MSBuild.exe";
            if (CreateProcessW(NULL, &cmd[0], NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
            {
                CloseHandle(hWritePipe);
                hWritePipe = NULL;

                char buffer[MAX_PATH] = { 0 };
                DWORD bytesRead = 0;
                std::string output;
                while (ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
                {
                    buffer[bytesRead] = '\0';
                    output += buffer;
                }
                WaitForSingleObject(pi.hProcess, 3000);
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);

                while (!output.empty() && (output.back() == '\r' || output.back() == '\n' || output.back() == ' '))
                {
                    output.pop_back();
                }

                if (!output.empty())
                {
                    int wlen = MultiByteToWideChar(CP_ACP, 0, output.c_str(), -1, NULL, 0);
                    if (wlen > 0)
                    {
                        std::wstring wResult(wlen, 0);
                        MultiByteToWideChar(CP_ACP, 0, output.c_str(), -1, &wResult[0], wlen);
                        if (!wResult.empty() && wResult.back() == L'\0') wResult.pop_back();

                        DWORD attribs = GetFileAttributesW(wResult.c_str());
                        if (attribs != INVALID_FILE_ATTRIBUTES && !(attribs & FILE_ATTRIBUTE_DIRECTORY))
                        {
                            return L"\"" + wResult + L"\"";
                        }
                    }
                }
            }
            if (hWritePipe) CloseHandle(hWritePipe);
            CloseHandle(hReadPipe);
        }
    }

    // 4. Exhaustive Multi-Drive Fallback Scan for MSBuild.exe
    std::vector<std::wstring> msbuildSubPaths = {
        L"Program Files\\Microsoft Visual Studio\\18\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe",
        L"Program Files\\Microsoft Visual Studio\\2022\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe",
        L"Program Files\\Microsoft Visual Studio\\2022\\Professional\\MSBuild\\Current\\Bin\\MSBuild.exe",
        L"Program Files\\Microsoft Visual Studio\\2022\\Enterprise\\MSBuild\\Current\\Bin\\MSBuild.exe",
        L"Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe",
        L"Program Files (x86)\\Microsoft Visual Studio\\2019\\Professional\\MSBuild\\Current\\Bin\\MSBuild.exe",
        L"Program Files (x86)\\Microsoft Visual Studio\\2019\\Enterprise\\MSBuild\\Current\\Bin\\MSBuild.exe",
        L"Microsoft Visual Studio\\2022\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe",
        L"Visual Studio\\2022\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe"
    };

    for (const auto& drive : drivePrefixes)
    {
        for (const auto& sub : msbuildSubPaths)
        {
            std::wstring fullPath = drive + sub;
            DWORD attribs = GetFileAttributesW(fullPath.c_str());
            if (attribs != INVALID_FILE_ATTRIBUTES && !(attribs & FILE_ATTRIBUTE_DIRECTORY))
            {
                return L"\"" + fullPath + L"\"";
            }
        }
    }

    return L"msbuild.exe";
}

static bool FindProjectOrSolutionFile(std::wstring& outProjectOrSolutionFile, std::wstring& outWorkingDir)
{
    wchar_t exePathBuf[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, exePathBuf, MAX_PATH);

    fs::path currentDir = fs::path(exePathBuf).parent_path();

    for (int i = 0; i < 6; ++i)
    {
        fs::path vcxproj = currentDir / "WinAPI-Coop-Survivor.vcxproj";
        if (fs::exists(vcxproj))
        {
            outProjectOrSolutionFile = L"\"" + vcxproj.wstring() + L"\"";
            outWorkingDir = currentDir.wstring();
            return true;
        }

        fs::path sln = currentDir / "WinAPI-Coop-Survivor.sln";
        if (fs::exists(sln))
        {
            outProjectOrSolutionFile = L"\"" + sln.wstring() + L"\"";
            outWorkingDir = currentDir.wstring();
            return true;
        }

        if (currentDir.has_parent_path() && currentDir.parent_path() != currentDir)
        {
            currentDir = currentDir.parent_path();
        }
        else
        {
            break;
        }
    }

    return false;
}

static bool RunMSBuild(const std::string& userCustomPath, std::string& outErrorMessage)
{
    std::wstring msbuildPath = FindMSBuildPath(userCustomPath);
    std::wstring projectFile;
    std::wstring workingDir;

    if (!FindProjectOrSolutionFile(projectFile, workingDir))
    {
        outErrorMessage = "Project/Solution file (WinAPI-Coop-Survivor.vcxproj / .sln) not found in parent directories!";
        return false;
    }

    fs::path buildsDir = fs::path(workingDir) / "Builds" / "Release";
    std::string outDirStr = buildsDir.generic_string();
    if (!outDirStr.empty() && outDirStr.back() != '/')
    {
        outDirStr += "/";
    }

    std::wstring wOutDir(outDirStr.begin(), outDirStr.end());
    std::wstring commandLine = msbuildPath + L" " + projectFile + L" /p:Configuration=Release /p:Platform=x64 /p:OutDir=\"" + wOutDir + L"\" /p:PreprocessorDefinitions=\"GAME_BUILD%3BNDEBUG%3B_CONSOLE\" /t:Build";

    SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    HANDLE hReadPipe = NULL, hWritePipe = NULL;
    if (!CreatePipe(&hReadPipe, &hWritePipe, &saAttr, 0))
    {
        outErrorMessage = "Failed to create output pipe for MSBuild.";
        return false;
    }
    SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW si = { sizeof(si) };
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdOutput = hWritePipe;
    si.hStdError = hWritePipe;
    si.wShowWindow = SW_HIDE;
    PROCESS_INFORMATION pi = { 0 };

    const wchar_t* pWorkingDir = workingDir.empty() ? NULL : workingDir.c_str();

    if (CreateProcessW(NULL, &commandLine[0], NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, pWorkingDir, &si, &pi))
    {
        CloseHandle(hWritePipe);
        hWritePipe = NULL;

        std::string buildLog;
        char buffer[4096] = { 0 };
        DWORD bytesRead = 0;
        while (ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            buildLog += buffer;
        }

        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD exitCode = 1;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        CloseHandle(hReadPipe);

        if (exitCode != 0)
        {
            std::ofstream logFile("msbuild_last_error.log");
            if (logFile.is_open())
            {
                logFile << buildLog;
                logFile.close();
            }

            std::string extractedError;
            std::istringstream stream(buildLog);
            std::string line;
            while (std::getline(stream, line))
            {
                if (line.find("error ") != std::string::npos || line.find("Error ") != std::string::npos || line.find("MSB") != std::string::npos)
                {
                    extractedError = line;
                    break;
                }
            }

            if (extractedError.empty())
            {
                outErrorMessage = "MSBuild failed (Exit code: " + std::to_string(exitCode) + "). See msbuild_last_error.log";
            }
            else
            {
                size_t first = extractedError.find_first_not_of(" \t\r\n");
                if (first != std::string::npos) extractedError = extractedError.substr(first);
                outErrorMessage = "Build error: " + extractedError;
            }
            return false;
        }
        return true;
    }
    else
    {
        if (hWritePipe) CloseHandle(hWritePipe);
        CloseHandle(hReadPipe);
        DWORD err = GetLastError();
        outErrorMessage = "Could not start MSBuild (Error: " + std::to_string(err) + "). Please set custom MSBuild path.";
        return false;
    }
}

MainMenuBarPanel::MainMenuBarPanel()
{
    LoadBuildSettings();
}

MainMenuBarPanel::~MainMenuBarPanel()
{
    if (m_buildThread.joinable())
    {
        m_buildThread.join();
    }
}

void MainMenuBarPanel::Initialize()
{
    GUISystem::GetInstance()->RegisterPanel(this);
}

void MainMenuBarPanel::Release()
{
    GUISystem::GetInstance()->UnRegisterPanel(this);
}

void MainMenuBarPanel::LoadBuildSettings()
{
    GetPrivateProfileStringA("BuildSettings", "MSBuildPath", "", m_customMSBuildPath, MAX_PATH, ".\\editor_build.ini");
}

void MainMenuBarPanel::SaveBuildSettings()
{
    WritePrivateProfileStringA("BuildSettings", "MSBuildPath", m_customMSBuildPath, ".\\editor_build.ini");
}

void MainMenuBarPanel::BuildAndRunRelease()
{
    if (m_buildState == BuildState::Building)
    {
        return;
    }

    SceneManager::GetInstance()->SaveActiveScene();

    if (m_buildThread.joinable())
    {
        m_buildThread.join();
    }

    m_buildState = BuildState::Building;
    m_buildStatusMessage = "Building Release (x64)...";

    std::string customPath = m_customMSBuildPath;
    m_buildThread = std::thread([this, customPath]() {
        std::string errorMsg;
        bool success = RunMSBuild(customPath, errorMsg);
        if (success)
        {
            m_buildStatusMessage = "Build succeeded! Launching Release...";
            RunReleaseExe();
            m_buildState = BuildState::Success;
        }
        else
        {
            m_buildStatusMessage = errorMsg;
            m_buildState = BuildState::Failed;
            m_showMSBuildModal = true;
        }
    });
}

void MainMenuBarPanel::RunReleaseExe()
{
    std::wstring projectFile, workingDir;
    std::vector<std::wstring> candidates;

    if (FindProjectOrSolutionFile(projectFile, workingDir))
    {
        fs::path base(workingDir);
        candidates.push_back((base / "Builds" / "Release" / "WinAPI-Coop-Survivor.exe").wstring());
        candidates.push_back((base / "WinAPI-Coop-Survivor" / "x64" / "Release" / "WinAPI-Coop-Survivor.exe").wstring());
        candidates.push_back((base / "x64" / "Release" / "WinAPI-Coop-Survivor.exe").wstring());
    }

    candidates.push_back(L"Builds\\Release\\WinAPI-Coop-Survivor.exe");
    candidates.push_back(L"..\\Builds\\Release\\WinAPI-Coop-Survivor.exe");
    candidates.push_back(L"x64\\Release\\WinAPI-Coop-Survivor.exe");
    candidates.push_back(L"WinAPI-Coop-Survivor\\x64\\Release\\WinAPI-Coop-Survivor.exe");
    candidates.push_back(L"..\\x64\\Release\\WinAPI-Coop-Survivor.exe");
    candidates.push_back(L"..\\WinAPI-Coop-Survivor\\x64\\Release\\WinAPI-Coop-Survivor.exe");

    std::wstring validExe = L"";
    for (const auto& cand : candidates)
    {
        DWORD attribs = GetFileAttributesW(cand.c_str());
        if (attribs != INVALID_FILE_ATTRIBUTES && !(attribs & FILE_ATTRIBUTE_DIRECTORY))
        {
            validExe = cand;
            break;
        }
    }

    if (validExe.empty())
    {
        m_buildStatusMessage = "Release exe not found! Build first.";
        return;
    }

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };

    fs::path exeDir = fs::path(validExe).parent_path();
    std::wstring wExeDir = exeDir.wstring();

    if (CreateProcessW(NULL, &validExe[0], NULL, NULL, FALSE, DETACHED_PROCESS, NULL, wExeDir.c_str(), &si, &pi))
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        m_buildStatusMessage = "Release build launched!";
    }
    else
    {
        m_buildStatusMessage = "Failed to launch Release exe.";
    }
}

void MainMenuBarPanel::OnDrawGUI()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Scene", "Ctrl+N"))
            {
                SceneManager::GetInstance()->CreateDefaultTemplateScene("NewScene");
            }
            ImGui::Separator();

            if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
            {
                SceneManager::GetInstance()->SaveActiveScene();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Build"))
        {
            if (ImGui::MenuItem("Build & Run Release (x64)", nullptr, false, m_buildState != BuildState::Building))
            {
                BuildAndRunRelease();
            }

            if (ImGui::MenuItem("Run Existing Release Exe"))
            {
                RunReleaseExe();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("MSBuild Settings..."))
            {
                m_showMSBuildModal = true;
            }

            ImGui::EndMenu();
        }

        ImGui::SameLine(ImGui::GetWindowWidth() * 0.45f);

        EnginePlayState playState = EngineKernel::GetInstance()->GetPlayState();

        if (playState == EnginePlayState::Edit)
        {
            if (ImGui::Button(" Play "))
            {
                SceneManager::GetInstance()->StartPlaySession();
            }
        }
        else
        {
            if (ImGui::Button(" Stop "))
            {
                SceneManager::GetInstance()->StopPlaySession();
            }
        }

        if (!m_buildStatusMessage.empty())
        {
            ImGui::SameLine();
            if (m_buildState == BuildState::Building)
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "[ %s ]", m_buildStatusMessage.c_str());
            else if (m_buildState == BuildState::Success)
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "[ %s ]", m_buildStatusMessage.c_str());
            else if (m_buildState == BuildState::Failed)
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "[ %s ]", m_buildStatusMessage.c_str());
            else
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "[ %s ]", m_buildStatusMessage.c_str());
        }

        ImGui::EndMainMenuBar();
    }

    if (m_showMSBuildModal)
    {
        ImGui::OpenPopup("MSBuild Path Configuration");
    }

    if (ImGui::BeginPopupModal("MSBuild Path Configuration", &m_showMSBuildModal, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Configure the path to MSBuild.exe for building Release configuration:");
        ImGui::Separator();

        ImGui::InputText("MSBuild.exe Path", m_customMSBuildPath, MAX_PATH);
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Example: C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe");

        ImGui::Spacing();
        if (ImGui::Button("Save & Apply", ImVec2(120, 0)))
        {
            SaveBuildSettings();
            m_showMSBuildModal = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            m_showMSBuildModal = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}