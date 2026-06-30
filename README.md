
# DirectX-Examples
![License](https://img.shields.io/badge/license-MIT-blue.svg)
![C++](https://img.shields.io/badge/language-C%2B%2B-blue.svg)
![Windows](https://img.shields.io/badge/platform-Windows-blue.svg)
![DirectX](https://img.shields.io/badge/API-DirectX%20%2F%20Windows%20Graphics-0078D7.svg)

A collection of C++ examples for learning and experimenting with Windows graphics and multimedia technologies such as Direct2D, DirectWrite, and other related APIs.

## Repository Structure

```text
DirectX-Examples/
├── .vscode/
├── Examples/
├── README.md
````

## Requirements

* A C/C++ compiler compatible with these examples, such as **MinGW-w64**
* **VS Code** or another code editor
* The required Windows headers and libraries for the APIs used by each example

## Setup

When opening the project in VS Code, create an `Output` folder in the root of the repository.

The `Launcher.bat` file expects that folder to exist so it can place compiled executables there.

If the folder does not exist, the project may still work, but a warning will be shown and the executables will be placed in the current folder instead.

## Build and Run

To run an example:

1. Open the example file or folder you want to test.
2. Build it using `Ctrl + Shift + B`, or the build task you have configured.
3. Run the executable generated in the `Output` folder.

Some examples require specific linker flags, such as:

* `-municode`
* `-ldwrite`
* `-ld2d1`

These flags are needed to link the Windows APIs used by the examples correctly.

If you are building a more complete application and do not want a console window to appear when launching the executable, you can use:

* `-mwindows`

If you want static linking for the C/C++ runtime, you can also use:

* `-static-libgcc`
* `-static-libstdc++`

## Notes

This repository is meant as a personal reference and a practical collection of examples.

The goal is to keep a place where you can quickly return to whenever you need to remember how to use a specific API, build option, or project setup.

## Contributing

Contributions are welcome.

You can add new examples, improve existing ones, or fix code and documentation.

Feel free to study the code, reuse it, or adapt it for your own projects.
