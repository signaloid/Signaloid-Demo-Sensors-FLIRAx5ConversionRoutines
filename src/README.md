# Source code:

## main.c
Implementation of the calculation of the calibrated sensor output for the FLIR sensor.

## kernel.c/h
Contains the calibration constants for the FLIR Ax5 conversion routine, the input/output
variable index enums, and the computational kernel (`FLIRAx5_calculateOutput`) that
implements the conversion formula. Dispatches to the UxHw-mode kernel
(`flir-ax5-uxhw.c/h`) or the Monte Carlo-mode kernel (`flir-ax5-monte-carlo.c/h`)
depending on the execution backend.

## flir-ax5-uxhw.c/h
UxHw-mode calculation kernel: performs a single distributional evaluation of the FLIR
Ax5 conversion routine via the UxHw API.

## flir-ax5-monte-carlo.c/h
Monte Carlo-mode calculation kernel: runs `numberOfMonteCarloIterations` independent
evaluations of the FLIR Ax5 conversion routine, used by the native Monte Carlo build.

## utilities.c/h
These contain utility methods for parsing, setting, and reporting
the usage of demo-specific command-line arguments of C/C++ demo applications.
These methods call similar methods from `common.c` for handling
command-line arguments common to all of our C/C++ demo applications.

## common.c/h
These contain utility methods for parsing, setting, and reporting
the usage of command-line arguments common to all of our C/C++ demo applications,
as well as other methods that we commonly make use across our
C/C++ demo applications, e.g., standard methods for I/O handling. These
source files are symlinks to the original files contained in the repository
[Signaloid-Demo-CommonUtilityRoutines](https://github.com/signaloid/Signaloid-Demo-CommonUtilityRoutines)
which is included as a submodule in `submodules/common`.

## uxhw.c/h
These contain methods that implement the probabilistic versions of the methods
in the UxHw API (e.g., `UxHwDoubleGaussDist`) and uses the GNU Scientific Library (GSL)
random number generators to achieve that. This allows building our C/C++ demo applications
natively (i.e., on conventional architectures) and running native Monte Carlo evaluations
of our C/C++ demo applications without modifying the source code.
These source files are symlinks to the original files and are contained in the repository
[Signaloid-Demo-UxHwCompatibilityForNativeExecution](https://github.com/signaloid/Signaloid-Demo-UxHwCompatibilityForNativeExecution)
which is included as a submodule in `submodules/compat`.

## config.mk
Signaloid cores use this file to identify the source codes they will use when
building the C/C++ demo application.

# To Build Natively on Non-Signaloid Platforms

Run `make local-build` from the repository root. This builds the
`demo-native-mc` executable at the repository root. See the top-level `README.md`'s
"Prerequisites" section for installing the required build dependencies.