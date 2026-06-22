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

## Design considerations
Why a tool for interactive measurement uncertainty analysis?
- To make the statistical process of measurement uncertainty easy for engineers
- Manual derivation of the required sensitivity coefficients can be infeasible for complex models
- The correlation between input parameters is often ignored due to the computational complexity

*Sigma* provides an easy, structured and automated workflow for measurement uncertainty analysis:
- The entered measurement function defines the relation between the input and output parameters
- The correlations between input parameters are taken into account properly
- The sensitivity coefficients are calculated automatically based on the partial derivatives of the measurement function
- The combined uncertainty and the components of the uncertainty budget are instantly displayed in the interface. The critical components can quickly be observed by a color scale.
- Monte Carlo simulation is available for more complex models. The visualization of the simulation output values provides insight into the probability distribution of the output parameter.
- Calculation projects can easily be saved or exported
- Uncertainty budget calculations can be integrated into an existing calculation pipeline using *Sigma*'s command-line interface

## Architecture
For maintainability and reusability, *Sigma* is structured into separate UI, Orchestration, and Core layers. The UI layer contains a graphical user interface (GUI) and a command-line interface (CLI). The GUI provides an interactive environment for defining parameters and immediate visualization of results. The Orchestration layer acts as the interface between the GUI/CLI and the Core systems. It orchestrates the execution of calculations, manages data flow between components, and handles file I/O. The Core layer handles the data storage and numerical computations.

### UI layer
- #### GUI (Qt Quick)
  - User interaction
  - Data input
  - Results visualization
  - Custom QML components
- #### CLI (Command-Line Interface)
  - Headless project automation
  - Monte Carlo simulation via command-line
  - JSON import/export

### Orchestration layer (C++)
- Orchestration of calculations and data flow
- Transaction-based undo/redo system
- Project file save/load (.sig)
- CSV export

### Core layer (C++)
- Data models
- Mathematical expression parser (ExprTk)
- Uncertainty budget calculations
- Correlated sampler
- Monte Carlo simulation
- Statistics

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

## Command-line interface
### Usage
- Windows `Sigma [options]`
- macOS `./Sigma.app/Contents/MacOS/Sigma [options]`

| Option | Description |
|:---------|:-------------|
| `-h`, `--help` | Displays help on command-line options |
| `-v`, `--version` | Displays version information |
| `-H`, `--headless` | Runs Sigma without the GUI |
| `--open <file>` | Opens a project from `<file>` |
| `--run <par>` | Runs Monte Carlo simulation for output parameter `<par>` |
| `--run-all` | Runs Monte Carlo simulation for all output parameters |
| `--save <file>` | Saves the project to `<file>` |
| `--export <file>` | Exports the results to CSV file |
| `--to-json` | Prints the project data to stdout in JSON format |
| `--from-json` | Loads the project data from stdin in JSON format |
| `--csvdigits <digits>` | Sets the CSV export significant digits (1 to 20) |
| `--mcdigits <digits>` | Sets the Monte Carlo significant digits (1 to 3) |
| `--mcbatchsize <size>` | Sets the Monte Carlo batch size (1e+02 to 1e+06) |
| `--mcmaxbatches <num>` | Sets the Monte Carlo maximum number of batches (1e+01 to 1e+05) |

### Example
Open a project, run all Monte Carlo simulations in headless mode, save the project, and export the results:
```bash
Sigma --headless --open project.sig --run-all --save project.sig --export export.csv
```

## JSON Format
*Sigma* projects are loaded and saved in JSON format and can also be imported and exported using the `--from-json` and `--to-json` command-line options. The JSON format can be used to create projects programmatically or integrate *Sigma* with other tools.

### Structure
A *Sigma* project JSON object contains three arrays:

```json
{
    "inputParameters":  [ ... ],
    "correlations":     [ ... ],
    "outputParameters": [ ... ]
}
```

### Input Parameters
Each input parameter defines a quantity with an associated probability distribution.

```json
{
    "Id":              "{ac7a4821-a1a4-4d72-88f5-f29b00779d41}",
    "name":            "X1",
    "unit":            "m",
    "nominalValue":    0,
    "stdUncertainty":  1,
    "distribution":    "normal",
    "DOFInfinite":     true,
    "DOF":             1
}
```

| Field            | Type    | Required | Description                                      |
|------------------|---------|----------|--------------------------------------------------|
| `name`           | string  | yes      | Unique identifier. Must start with a letter and cannot be a mathematical operator or constant (e.g. `sin`, `pi`) |
| `nominalValue`   | number  | yes      | Best estimate of the quantity                    |
| `stdUncertainty` | number  | yes      | Standard uncertainty (≥0)                        |
| `distribution`   | string  | yes      | Probability distribution (see below)             |
| `DOFInfinite`    | boolean | yes      | `true` if degrees of freedom are infinite        |
| `DOF`            | integer | no       | Degrees of freedom. Required when `DOFInfinite` is `false` |
| `unit`           | string  | no       | Unit of measurement                              |
| `Id`             | string  | no       | UUID in `{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}` format. Required when referenced in a correlation |

#### Distributions

| Value         | Description                  |
|---------------|------------------------------|
| `normal`      | Normal (Gaussian) distribution |
| `uniform`     | Uniform (rectangular) distribution |
| `triangular`  | Triangular distribution      |
| `arcsine`     | Arcsine distribution         |
| `student`     | Student's t-distribution     |
| `none`        | Constant — no distribution   |

### Correlations

Correlations define the statistical dependence between pairs of input parameters. The input parameter Ids can be entered in arbitrary order. Only non-zero correlations need to be specified.

```json
{
    "Id":               "{bbf6947c-fd34-4b9e-b367-fbca27143be3}",
    "IdInputParameterA": "{ac7a4821-a1a4-4d72-88f5-f29b00779d41}",
    "IdInputParameterB": "{856a592f-4562-41ce-bb79-f1c57dbfba21}",
    "correlation":       0.5
}
```

| Field               | Type   | Required | Description                                      |
|---------------------|--------|----------|--------------------------------------------------|
| `IdInputParameterA` | string | yes      | `Id` of the first input parameter                |
| `IdInputParameterB` | string | yes      | `Id` of the second input parameter               |
| `correlation`       | number | yes      | Correlation coefficient in the range `[-1, 1]`  |
| `Id`                | string | no       | UUID of the correlation                          |

### Output Parameters

Each output parameter defines a measurand expressed as a formula of input parameters.

```json
{
    "Id":         "{a377bfca-38a4-4177-918c-0bc554859a3c}",
    "name":       "Y1",
    "unit":       "m",
    "formula":    "X1 + X2",
    "confidence": 0.95
}
```

| Field        | Type   | Required | Description                                                  |
|--------------|--------|----------|--------------------------------------------------------------|
| `name`       | string | yes      | Unique identifier. Must be at least one character long       |
| `formula`    | string | yes      | Mathematical expression using input parameter names          |
| `confidence` | number | yes      | Coverage probability in the range `(0, 1)`, e.g. `0.95` for 95% |
| `unit`       | string | no       | Unit of measurement                                          |
| `Id`         | string | no       | UUID of the output parameter                                 |

### Example

```json
{
    "inputParameters": [
        {
            "name":           "X1",
            "unit":           "m",
            "nominalValue":   0,
            "stdUncertainty": 1,
            "distribution":   "normal",
            "DOFInfinite":    true,
            "Id":             "{ac7a4821-a1a4-4d72-88f5-f29b00779d41}"
        },
        {
            "name":           "X2",
            "unit":           "m",
            "nominalValue":   2,
            "stdUncertainty": 3,
            "distribution":   "uniform",
            "DOFInfinite":    false,
            "DOF":            10,
            "Id":             "{856a592f-4562-41ce-bb79-f1c57dbfba21}"
        }
    ],
    "correlations": [
        {
            "IdInputParameterA": "{ac7a4821-a1a4-4d72-88f5-f29b00779d41}",
            "IdInputParameterB": "{856a592f-4562-41ce-bb79-f1c57dbfba21}",
            "correlation":       0.5
        }
    ],
    "outputParameters": [
        {
            "name":       "Y1",
            "unit":       "m",
            "formula":    "X1 + X2",
            "confidence": 0.95
        }
    ]
}
```

### Notes

- UUIDs are generated automatically by *Sigma* when saving a project. When creating JSON manually, UUIDs can be omitted unless correlations are defined, in which case `Id` must be specified for the referenced input parameters.
- The `formula` field supports standard mathematical operators (`+`, `-`, `*`, `/`, `^`) and functions (`sin()`, `cos()`, `sqrt()`, `abs()`, `log()` etc.) via the [ExprTk](https://www.partow.net/programming/exprtk/index.html) expression parser.
- Mathematical constants such as `pi`, `epsilon` and `inf` are reserved and cannot be used as input parameter names.
