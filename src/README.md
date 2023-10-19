
# MB-08 GUI

## Compilation Instructions
1. Environment Setup
    a. Install [Visual Studio Code](code.visualstudio.com)
    b. [Clone](https://learn.microsoft.com/en-us/azure/developer/javascript/how-to/with-visual-studio-code/clone-github-repository?tabs=create-repo-command-palette%2Cinitialize-repo-activity-bar%2Ccreate-branch-command-palette%2Ccommit-changes-command-palette%2Cpush-command-palette) the MB-08 GUI [repository](github.com/cbarrett-sbir/mb-GUI-2)
	c. Install the [PlatformIO]( platformio.org/install/ide?install=vscode) extension for VS Code
	d. Installation may take a minute. Open the alien icon in the left sidebar once it completes.
	e. From the PIO Homepage, open the mb-GUI-2 project which should appear after the ``platformio.ini`` file in the cloned repository is detected
3. Build and Upload
	a. Open a C++ file in the src directory
	b. A &rarr; icon will appear at the top right of the screen. This is the upload button
	c. Press it to compile and upload the program to a connected teesny. PIO will automatically find the connected device
