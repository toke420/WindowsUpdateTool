# WindowsUpdateTool
An executable you can use for managing various windows updates tasks.

A console based application written in C++ that runs powershell scripts to accomplish the selected tasks.

The application must be run with administrative rights and was written using chatgpt and copilot. 

This application was written for Windows 11 but works on Windows 10 also.

Example:

=== WindowsUpdateTool-v2.1.3 ===
1. Download and install all Windows updates then reboot if required.
2. Download and install driver updates then reboot if required.
3. Download and install firmware updates then reboot if required.
4. Download but do not install all Windows updates.
5. Download and install individual updates.
6. Clean Windows Update certificate store to resolve failed update issues.
7. Perform a full reset of all Windows Update components.
0. Exit.

Enter your choice:

----------------------------------------------------------------------

#Known Issues
Option 7 will produced some errors when trying to clean the SoftwareDistribution and Catroot2 directories. This seems to break RDP temporarily but can be resolved by disabling Remote Desktop, restarting windows, then re-enabling Remote Desktop. Therefore you should only run this if you have alternative console access like on a virtual machine or when physically infront of the computer or server.
