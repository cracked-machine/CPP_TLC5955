## TLC5955 48 Channel RGB LED driver library

TLC5955 datasheet can be found [here](https://www.ti.com/product/TLC5955)

See [LedManager](https://github.com/cracked-machine/StepSequencer_SW/blob/main/main_app/src/led_manager.cpp) for example of how to use this driver.

### Design

![SSD1306 Class Diagram](docs/TLC5955_Class_Diagram.png)


![cached_image](http://www.plantuml.com/plantuml/svg/xLNVQzim47xlNt5Fmu6K_XZzKPE4Dcu9eGa9jGlRav1jnHQJ9IETsuQo_tsod7HBrzGRb39ONqBE-kwxx-Ixp-S6gSQg43lzFWHAebO2BYbIc51CSkZtTwQKoukoXDq1H9FWwEpeY5ngViUq3vePXbxdDOW5a0Ir84cHjYDBcgPSPYJcQDhHYqeco9LqtB6oobDJwXAxu08iCctMWLvOiVagOAQyBrR_XKdl1BRMoeIAgI2nvbcEaXdZYbPSuYb1Y4N5kh2ECisO_1bgbhRI5_I199NMJEBR4gVA4YfB_7HJX0hnn6_luLMAKgkaRbQpZ1jashwKJKYTn8gOfQH2BWo9UTsCvAKFzc-j01M9byYerdJPEpk-xSB9yUdjm3EO-dxpTAMc2_Pu09-GwimckhCk94eQ18Tk67iklwLpcJPZPrCyr1LxINSOJJyV-d393Oq5Wt0-FWHlFZpPcmzFNy5VIGTV0O4Q6Ct7CvXLM5PejkT5Aiji7kOkrvlEmd6dPl32uBhZE0mib0OkEN8g85cjet2RQ1i1bmE68UHs35ibFk-ezSPCoTsxCbuxzoWC9hFW6hp9X4JZAGa2qfm4EPMI2LjEA5N2VSujsh42AbV7tquUskDQTfFWo6r48asbAJZMpcGRiQ0rE-NeBYtwSWKBGJD_vz5PsdVuupaLWjGqnD8GV62afjckCCXPyWrgKU4DgQjowUvvTn3UE66FzkUtwhtTRTKZNZ1LuPFidYtLKFXmiByFbNdUwZ3y-XQjncRvDwtQgk_GQikMlZntzcXZyDesS3uysvjSH51owMZgrICXmsQ3vrpkhH_UCFFBOplIzMTofDdI9DGIUQEmy-PVosnDls7M1yV_Yrilg33_a5qhAPQmuT8R9fsPb29zqQE5kdzNY_wmw0-B_h3eSwksAiGF)

### Usage

[For example on how to use this class](https://godbolt.org/z/rx53GcrsE)

### Running Units Tests on X86

Running `cmake` in the root directory will invoke the tests.
It is recommended to use VSCode and "CMake Tools" extension with Test Suite, but if you insist on using the CLI:

- Configure the testsuite:
`cmake --no-warn-unused-cli -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_C_COMPILER:FILEPATH=/bin/x86_64-linux-gnu-gcc-10 -Bbuild -G Ninja .`
- Build the testsuite
`cmake --build build --config Debug --target all --`
- Run the testsuite
`./build/test_suite`
- Clean the testsuite
`cmake --build build --config Debug --target clean --`

### Adding this library to your STM32 Project

Include this repo into your project as a submodule and add the following line to your top-level CMakeFiles.txt:

`add_subdirectory(embedded_utils)`

This assumes your project's top-level CMakeFiles.txt is already configured for STM32 platform.

### Debug

`GDB` is enabled in VSCode (F5)

### Code Coverage report

Run the "coverage report" task. Results can be found in `coverage` directory.


