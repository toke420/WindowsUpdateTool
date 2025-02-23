#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <windows.h>
#include <cstdio>
#include <limits>

void executePowerShellCommand(const std::string& command, bool isScript = false) {
    std::string fullCommand;
    if (isScript) {
        fullCommand = "powershell.exe -NoProfile -ExecutionPolicy Bypass -File \"" + command + "\"";
    } else {
        fullCommand = "powershell.exe -NoProfile -ExecutionPolicy Bypass -Command \"" + command + "\"";
    }
    std::cout << "\nExecuting Command:\n" << fullCommand << "\n";
    int result = system(fullCommand.c_str());
    if (result != 0) {
        std::cerr << "Command failed with return code: " << result << "\n";
    }
}

void createLogsDirectory() {
    DWORD ftyp = GetFileAttributesA("C:\\Logs");
    if (ftyp == INVALID_FILE_ATTRIBUTES) {
        system("mkdir C:\\Logs");
    }
}

bool isModuleInstalled() {
    std::string command = "if (Get-Module -ListAvailable -Name PSWindowsUpdate) { exit 0 } else { exit 1 }";
    int result = system(("powershell.exe -NoProfile -ExecutionPolicy Bypass -Command \"" + command + "\"").c_str());
    return (result == 0);
}

void installModule() {
    std::cout << "\nInstalling required module: PSWindowsUpdate\n";
    executePowerShellCommand("Install-Module -Name PSWindowsUpdate -Force -Confirm:$false");
}

void resetWindowsUpdateComponents() {
    std::cout << "\nWARNING: Running this option may disrupt Remote Desktop (RDP) connections.\n";
    std::cout << "It's recommended to only run this option when you are physically in front of the computer.\n";
    std::cout << "Do you wish to proceed? (y/n): ";
    char proceed;
    std::cin >> proceed;
    if (proceed != 'y' && proceed != 'Y') {
        std::cout << "\nOperation canceled by the user.\n";
        return;
    }

    std::cout << "\nStarting Windows Update components reset...\n";

    // Step 1: Stop services
    executePowerShellCommand(
        "net stop wuauserv; "
        "net stop cryptSvc; "
        "net stop bits; "
        "net stop msiserver"
    );

    // Step 2: Remove SoftwareDistribution and Catroot2 contents
    executePowerShellCommand(
        "Remove-Item -Path 'C:\\Windows\\SoftwareDistribution\\*' -Recurse -Force; "
        "Remove-Item -Path 'C:\\Windows\\System32\\catroot2\\*' -Recurse -Force"
    );

    // Step 3: Reset Windows Update components
    executePowerShellCommand(
        "@\"\n"
        "sc.exe sdset bits D:(A;;CCLCSWRPWPDTLOCRRC;;;SY)\n"
        "   (A;;CCDCLCSWRPWPDTLOCRSDRCWDWO;;;BA)\n"
        "   (A;;CCLCSWRPWPDTLOCRRC;;;AU)\n"
        "   (A;;CCLCSWRPWPDTLOCRRC;;;PU);\n"
        "sc.exe sdset wuauserv D:(A;;CCLCSWRPWPDTLOCRRC;;;SY)\n"
        "   (A;;CCDCLCSWRPWPDTLOCRSDRCWDWO;;;BA)\n"
        "   (A;;CCLCSWRPWPDTLOCRRC;;;AU)\n"
        "   (A;;CCLCSWRPWPDTLOCRRC;;;PU)\n"
        "\"@"
    );

    // Step 4: Register DLLs
    executePowerShellCommand(
        "@\"\n"
        "regsvr32.exe /s wuaueng.dll\n"
        "regsvr32.exe /s wuapi.dll\n"
        "regsvr32.exe /s wuaueng1.dll\n"
        "regsvr32.exe /s wucltui.dll\n"
        "regsvr32.exe /s wups.dll\n"
        "regsvr32.exe /s wups2.dll\n"
        "regsvr32.exe /s wuwebv.dll\n"
        "regsvr32.exe /s atl.dll\n"
        "regsvr32.exe /s msxml3.dll\n"
        "regsvr32.exe /s msxml6.dll\n"
        "regsvr32.exe /s softpub.dll\n"
        "regsvr32.exe /s wintrust.dll\n"
        "regsvr32.exe /s dssenh.dll\n"
        "regsvr32.exe /s rsaenh.dll\n"
        "regsvr32.exe /s gpkcsp.dll\n"
        "regsvr32.exe /s sccbase.dll\n"
        "regsvr32.exe /s slbcsp.dll\n"
        "regsvr32.exe /s cryptdlg.dll\n"
        "regsvr32.exe /s oleaut32.dll\n"
        "regsvr32.exe /s ole32.dll\n"
        "regsvr32.exe /s shell32.dll\n"
        "regsvr32.exe /s initpki.dll\n"
        "regsvr32.exe /s wucltui.dll\n"
        "regsvr32.exe /s wuweb.dll\n"
        "regsvr32.exe /s qmgr.dll\n"
        "regsvr32.exe /s qmgrprxy.dll\n"
        "\"@"
    );

    // Step 5: Reset network configurations
    executePowerShellCommand(
        "netsh winsock reset; "
        "netsh winsock reset proxy; "
        "netsh advfirewall reset"
    );

    // Step 6: Start services
    executePowerShellCommand(
        "net start wuauserv; "
        "net start cryptSvc; "
        "net start bits; "
        "net start msiserver"
    );

    std::cout << "\nWindows Update components have been reset successfully.\n";
}

int main() {
    // Ensure the PSWindowsUpdate module is installed
    if (!isModuleInstalled()) {
        installModule();
    }

    createLogsDirectory();

    int choice;

    do {
        system("cls"); // Clear the console for better readability
        std::cout << "=== WindowsUpdateTool-v2.1.3 ===\n";
        std::cout << "1. Download and install all Windows updates then reboot if required.\n";
        std::cout << "2. Download and install driver updates then reboot if required.\n";
        std::cout << "3. Download and install firmware updates then reboot if required.\n";
        std::cout << "4. Download but do not install all Windows updates.\n";
        std::cout << "5. Download and install individual updates.\n";
        std::cout << "6. Clean Windows Update certificate store to resolve failed update issues.\n";
        std::cout << "7. Perform a full reset of all Windows Update components.\n";
        std::cout << "0. Exit.\n";
        std::cout << "\nEnter your choice: ";

        std::cin >> choice;

        // Validate input
        if (std::cin.fail()) {
            std::cin.clear(); // Clear error flags
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard invalid input
            std::cout << "\nInvalid choice. Please enter a number from 0 to 7.\n";
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
            continue; // Restart loop
        }

        // Clear any leftover input
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice) {
            case 1: {
                // Install all Windows updates
                executePowerShellCommand(
                    "Import-Module PSWindowsUpdate; "
                    "Get-WindowsUpdate -Install -AcceptAll -AutoReboot -Verbose | "
                    "Out-File -FilePath 'C:\\Logs\\WindowsUpdates.log' -Append"
                );
                break;
            }
            case 2: {
                // Install driver updates
                executePowerShellCommand(
                    "Import-Module PSWindowsUpdate; "
                    "Get-WindowsUpdate -Category Drivers -Install -AcceptAll -AutoReboot -Verbose | "
                    "Out-File -FilePath 'C:\\Logs\\DriverUpdates.log' -Append"
                );
                break;
            }
            case 3: {
                // Install firmware updates
                executePowerShellCommand(
                    "Import-Module PSWindowsUpdate; "
                    "Get-WindowsUpdate -Category Firmware -Install -AcceptAll -AutoReboot -Verbose | "
                    "Out-File -FilePath 'C:\\Logs\\FirmwareUpdates.log' -Append"
                );
                break;
            }
            case 4: {
                // Download but do not install updates
                executePowerShellCommand(
                    "Import-Module PSWindowsUpdate; "
                    "Get-WindowsUpdate -Download -AcceptAll -Verbose | "
                    "Out-File -FilePath 'C:\\Logs\\WindowsUpdatesDownload.log' -Append"
                );
                break;
            }
            case 5: {
                // Option 5: Download and install individual updates
                std::cout << "\nFetching list of available updates...\n";

                // Create a temporary PowerShell script to list updates
                std::string scriptPath = "C:\\Logs\\ListUpdates.ps1";
                std::ofstream scriptFile(scriptPath);
                if (!scriptFile) {
                    std::cout << "Failed to create PowerShell script.\n";
                    break;
                }

                // Write the PowerShell script content
                scriptFile << "Import-Module PSWindowsUpdate\n";
                scriptFile << "$Updates = Get-WindowsUpdate -Verbose\n";
                scriptFile << "$counter = 0\n";
                scriptFile << "$Updates | ForEach-Object {\n";
                scriptFile << "    \"${counter}: $($_.Title)\"\n";
                scriptFile << "    $counter++\n";
                scriptFile << "} | Out-File -FilePath 'C:\\Logs\\AvailableUpdates.txt' -Encoding UTF8\n";
                scriptFile.close();

                // Execute the PowerShell script
                executePowerShellCommand(scriptPath, true);

                // Read the updates from the file and display them
                std::string tempFile = "C:\\Logs\\AvailableUpdates.txt";
                std::ifstream infile(tempFile);
                if (!infile) {
                    std::cout << "Failed to read the list of updates.\n";
                    break;
                }

                std::string line;
                while (std::getline(infile, line)) {
                    std::cout << line << "\n";
                }
                infile.close();

                // Prompt for index number or 'x' to return to main menu
                std::string input;
                std::cout << "\nEnter the index number of the update to install or press 'x' to return to the main menu: ";
                std::cin >> input;

                if (input == "x" || input == "X") {
                    // Return to main menu
                    break;
                } else {
                    try {
                        // Convert input to integer
                        int index = std::stoi(input);

                        // Create a PowerShell script to install the selected update
                        std::string installScriptPath = "C:\\Logs\\InstallUpdate.ps1";
                        std::ofstream installScriptFile(installScriptPath);
                        if (!installScriptFile) {
                            std::cout << "Failed to create PowerShell script.\n";
                            break;
                        }

                        // Write the PowerShell script content
                        installScriptFile << "Import-Module PSWindowsUpdate\n";
                        installScriptFile << "$Updates = Get-WindowsUpdate -Verbose\n";
                        installScriptFile << "$SelectedUpdate = $Updates[" << index << "]\n";
                        installScriptFile << "Install-WindowsUpdate -Updates $SelectedUpdate -AcceptAll -Verbose | Out-File -FilePath 'C:\\Logs\\IndividualUpdate.log' -Append\n";
                        installScriptFile.close();

                        // Execute the PowerShell script to install the update
                        executePowerShellCommand(installScriptPath, true);

                        // Optionally delete the install script
                        remove(installScriptPath.c_str());
                    } catch (std::exception& e) {
                        std::cout << "\nInvalid input. Returning to the main menu.\n";
                    }
                }

                // Optionally delete the temporary script and output files
                remove(scriptPath.c_str());
                remove(tempFile.c_str());

                break;
            }
            case 6: {
                // Option 6: Clean Windows Update certificate store
                std::cout << "\nCleaning Windows Update certificate store...\n";

                // Create a temporary PowerShell script
                std::string scriptPath = "C:\\Logs\\CleanCertStore.ps1";
                std::ofstream scriptFile(scriptPath);
                if (!scriptFile) {
                    std::cout << "Failed to create PowerShell script.\n";
                    break;
                }

                // Write the PowerShell script content
                scriptFile << "net stop wuauserv\n";
                scriptFile << "net stop cryptSvc\n";
                scriptFile << "net stop bits\n";
                scriptFile << "net stop msiserver\n";
                scriptFile << "Remove-Item -Path \"$Env:ALLUSERSPROFILE\\Microsoft\\Network\\Downloader\\*\" -Recurse -Force\n";
                scriptFile << "Remove-Item -Path \"$Env:SystemRoot\\System32\\catroot2\\*\" -Recurse -Force\n";
                scriptFile << "net start wuauserv\n";
                scriptFile << "net start cryptSvc\n";
                scriptFile << "net start bits\n";
                scriptFile << "net start msiserver\n";
                scriptFile.close();

                // Execute the PowerShell script
                executePowerShellCommand(scriptPath, true);

                // Optionally delete the temporary script
                remove(scriptPath.c_str());

                std::cout << "\nWindows Update certificate store cleaned successfully.\n";
                break;
            }
            case 7: {
                // Option 7: Reset Windows Update components with warning
                resetWindowsUpdateComponents();
                break;
            }
            case 0: {
                std::cout << "\nExiting application.\n";
                break;
            }
            default: {
                std::cout << "\nInvalid choice. Please try again.\n";
                break;
            }
        }

        if (choice != 0) {
            std::cout << "\nOperation completed. Press Enter to continue...";
            std::cin.get(); // Wait for user input
        }

    } while (choice != 0);

    return 0;
}
