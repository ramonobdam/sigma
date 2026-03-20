# Sigma - Measurement Uncertainty Toolkit 
*Sigma – Measurement Uncertainty Toolkit* is a desktop application for interactive measurement uncertainty analysis. 
It implements the classical GUM (Guide to the expression of Uncertainty in Measurement) method ([JCGM 100:2008](https://doi.org/10.59161/JCGM100-2008E)) for combined uncertainty and uncertainty budgets, 
and also offers a Monte Carlo simulation approach ([JCGM 101:2008](https://doi.org/10.59161/JCGM101-2008)) for more complex cases. 
Arbitrary measurement functions can be evaluated, and both independent and correlated input parameters are supported.

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="resources\screenshots\Sigma_H1_main_dark.png">
  <source media="(prefers-color-scheme: light)" srcset="resources\screenshots\Sigma_H1_main_light.png">
  <img alt="Sigma screenshot" src="screenshot-light.png">
</picture>

## Demo video
Watch a short demo video on [YouTube](https://youtu.be/Oq3ydIS2QOc)

## Architecture
- UI
  - Qt Quick
  - Customized QML components


- C++ backend
  - Data models
  - Mathematical expression parser 
  - Uncertainty budget calculations 
  - Correlated sampler
  - Monte Carlo simulation

## Installation 
Installers for Windows and macOS are bundled with the release. See the [latest release](https://github.com/ramonobdam/sigma/releases) on GitHub. 

Compiling your own version of *Sigma* requires:
- Qt 6.9+, including Qt Graphs ([Qt Installer](https://www.qt.io/development/download))
- Qt Creator or the command-line tools CMake 3.16+ and Ninja
- C++ compiler:
  - Windows: MSVC 2019/2022 ([Build Tools for Visual Studio](https://visualstudio.microsoft.com/downloads)) or MinGW
  - macOS: Xcode command-line tools (xcode-select --install)

## Quick start guide
1. Open *Sigma*
2. Define the input parameters by clicking 'New' in the Input parameters panel or choosing 'Input parameters > Add new...' (Ctrl+Shift+I) from the main menu. See the GUM for details on how to set the input estimate, standard uncertainty, probability distribution and degrees of freedom.
3. Create at least one output parameter by clicking 'New' in the Output parameters panel or choosing 'Output parameters > Add new...' (Ctrl+Shift+O) from the main menu. The measurement function of the output parameter defines the mathematical relation between the output value and the input parameters. The level of confidence specifies the required coverage probability of the expanded uncertainty interval. In other words, it gives the fraction of all possible output values that is contained in the expanded uncertainty interval.
4. The combined uncertainty and uncertainty budget are shown in the results panel on the bottom of the interface.
5. Choose 'Monte Carlo simulation > Start' (Ctrl+R) from the main menu to start the Monte Carlo simulation. The results can be found in the Monte Carlo simulation panel on the right.
6. To save a project, choose 'Project > Save...' (Ctrl+S) from the main menu, enter the filename and localion, and click 'Save project'. The file extension of Sigma projects is 'sig'.
7. The calculation results can be exported to CSV (Comma Separated Values) file by choosing 'Project > Export results to CSV file...' (Ctrl+E) from the main menu. Subsequently, enter the filename and localion, and click 'Save CSV'.
8. Application settings can be accessed via 'Sigma > Settings...' on Windows or 'Sigma > Preferences...' on macOS.

## Demo projects
The [demo projects](https://github.com/ramonobdam/sigma/tree/main/demo_projects) folder contains *Sigma* project files for the calculation examples given in GUM parts [JCGM 100:2008](https://doi.org/10.59161/JCGM100-2008E) and [JCGM 101:2008](https://doi.org/10.59161/JCGM101-2008). Note that the demo projects are also added to the installation folder when you use the installer.

*Sigma* projects can be opened using 'Project > Open...' (Ctrl+O) from the main menu.
