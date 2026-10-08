# AGENTS.md

## Project

Classical Code Assistant is a native Win32 AI client targeting Windows 2000.

The project intentionally uses an old-school development stack.

The primary goal is not to create the most modern application possible.

The primary goal is:

> Build a useful AI client that can actually run on Windows 2000.

---

## Target Environment

Primary target:

- Windows 2000
- Windows 2000 Professional
- Windows 2000 Server
- Windows 2000 Datacenter Server

Development may happen on modern operating systems, but compatibility with
the target environment must remain a priority.

---

## Compiler

The project is intended to support old MinGW / GCC toolchains and old
Dev-C++ environments.

Do not assume a modern compiler.

Avoid unnecessary use of modern C++ language features.

Prefer code that can be compiled as old-style C++.

---

## Language

Primary language:

```text
C++
````

GUI:

```text
Win32 API
```

Configuration:

```text
Windows INI APIs
```

The project does not use:

* Qt
* Electron
* .NET
* MFC
* Chromium
* wxWidgets
* other large GUI frameworks

unless explicitly approved.

---

## Coding Rules

### Keep it simple

Prefer straightforward code.

Avoid abstractions that exist only to make a small piece of code look modern.

Prefer:

```cpp
char buffer[256];
```

when a fixed-size buffer is sufficient.

Do not introduce complex containers or frameworks without a reason.

### Compatibility first

Before using a Windows API, consider whether it exists on Windows 2000.

Do not blindly use APIs introduced in later versions of Windows.

### Old MinGW compatibility

The project may use compatibility workarounds for incomplete or outdated
MinGW headers.

For example, if an old MinGW header does not define a Win32 structure,
a local compatible structure may be used when safe.

Do not replace a working compatibility workaround with a modern API merely
because the modern API is cleaner.

---

## C++ Standard

Use an old-compatible subset of C++.

Avoid unless explicitly required:

* C++11
* C++14
* C++17
* C++20
* C++23

In particular, avoid:

```cpp
auto
nullptr
lambda expressions
range-based for
std::thread
modern filesystem APIs
```

unless compatibility has been verified and the project explicitly decides
to allow them.

---

## Comments

Source code comments should primarily be written in English.

Comments should explain:

* compatibility constraints;
* non-obvious Win32 behavior;
* design decisions;
* protocol behavior.

Do not add comments that merely restate obvious code.

---

## File Organization

Current source layout:

```text
src/
├── main.cpp
├── gui.cpp
├── gui.h
├── config.cpp
└── config.h
```

Responsibilities:

### `main.cpp`

Application entry point.

Responsibilities:

* initialize common controls;
* initialize configuration;
* start the GUI;
* run the Windows message loop.

Do not put large GUI implementations here.

### `gui.h`

GUI declarations and public GUI interfaces.

### `gui.cpp`

Win32 GUI implementation.

Responsibilities:

* main window;
* tabs;
* chat controls;
* settings controls;
* provider selection;
* window resizing;
* UI events.

### `config.h`

Configuration structures and public configuration APIs.

### `config.cpp`

INI file handling and provider configuration.

Do not put GUI code in this file.

---

## Configuration

Use the Windows INI APIs:

```cpp
GetPrivateProfileStringA()
WritePrivateProfileStringA()
```

Configuration should be stored next to the executable.

Example:

```ini
[General]
ActiveProvider=0

[Provider0]
Name=Local Gateway
Endpoint=http://192.168.1.100:8000/v1/chat
ApiKey=
Model=default
SystemPrompt=You are a helpful AI assistant.
```

Never commit real API keys.

`config.ini` must remain in `.gitignore`.

---

## Provider Model

The client supports multiple provider configurations.

A provider contains:

```text
Name
Endpoint
ApiKey
Model
SystemPrompt
```

The GUI should allow the user to:

* add a provider;
* select a provider;
* edit a provider;
* save a provider;
* delete a provider.

When deleting a provider, the application should avoid leaving the
configuration in an invalid state.

At least one provider should normally remain configured.

---

## GUI

The GUI should look appropriate on Windows 2000.

Preferred controls:

* standard Win32 windows;
* EDIT controls;
* BUTTON controls;
* COMBOBOX controls;
* TAB controls;
* progress bars;
* standard message boxes.

Do not introduce a web-based GUI.

The UI should remain usable at common 800x600 and 1024x768 resolutions.

Window resizing should be handled where practical.

---

## Common Controls

The application may use:

```text
comctl32
```

The linker flag is:

```text
-lcomctl32
```

Do not use:

```cpp
#pragma comment(lib, "comctl32.lib")
```

because the primary toolchain is old MinGW rather than MSVC.

If an old compiler reports missing common-control structures, prefer a small
compatibility definition when appropriate.

---

## Networking

The client is designed to communicate with a Linux AI gateway.

Preferred architecture:

```text
Windows 2000
    |
    | HTTP
    v
Linux FastAPI Gateway
    |
    | HTTPS
    v
AI Provider
```

The Windows 2000 client should not be responsible for modern TLS complexity
when it can reasonably be handled by the gateway.

Do not expose an unauthenticated gateway to the public Internet.

---

## AI Provider Support

The gateway may eventually support:

* OpenAI
* DeepSeek
* Ollama
* other compatible providers

The Windows client should preferably communicate with one stable gateway
interface instead of implementing every provider's API independently.

---

## Error Handling

Do not silently ignore important failures.

For user-facing failures, prefer simple Windows message boxes or status text.

Examples:

```text
Unable to save configuration.
Unable to connect to gateway.
Invalid provider configuration.
Request timed out.
```

Avoid crashes caused by invalid configuration whenever reasonably possible.

---

## Security

Never hard-code real API keys.

Never commit:

```text
config.ini
```

if it contains credentials.

Do not add code that disables security mechanisms merely to make a modern
service work.

The legacy HTTP gateway architecture is intended for controlled environments.

---

## Build

A typical build command is:

```bash
g++ src/main.cpp src/gui.cpp src/config.cpp \
    -o classical-code-assistant.exe \
    -mwindows \
    -lcomctl32
```

When compiling individual source files:

```bash
g++ -c src/main.cpp
g++ -c src/gui.cpp
g++ -c src/config.cpp
```

Then link:

```bash
g++ main.o gui.o config.o \
    -o classical-code-assistant.exe \
    -mwindows \
    -lcomctl32
```

Do not put `-lcomctl32` into a compile-only command using `-c`.

---

## Testing

When modifying the application, test at least:

1. Application startup.
2. Main window creation.
3. Tab switching.
4. Window resizing.
5. Provider selection.
6. Provider creation.
7. Provider editing.
8. Provider deletion.
9. Configuration saving.
10. Configuration loading.
11. Invalid configuration handling.

If possible, test the executable directly inside a Windows 2000 VM.

---

## Git Workflow

The repository may be maintained from a modern machine.

The Windows 2000 environment does not need to contain Git.

Typical workflow:

```text
Modern PC
    |
    | GitHub / Git
    v
Repository
    |
    v
Windows 2000 VM
    |
    | compile / run / test
    v
Feedback
    |
    v
Modern PC
```

Do not add Git as a dependency to the Windows 2000 application.

---

## Dependencies

Before adding a dependency, ask:

1. Can Win32 already provide this functionality?
2. Can the Linux gateway handle this functionality?
3. Is the dependency compatible with Windows 2000?
4. Is it compatible with the old compiler?
5. Is the dependency actually necessary?

Prefer fewer dependencies.

---

## Pull Request Expectations

A change should:

* preserve the project's architecture;
* preserve Windows 2000 compatibility;
* avoid unnecessary dependencies;
* avoid modern language features unless justified;
* remain understandable;
* include a short explanation when introducing compatibility code.

Do not rewrite large parts of the project just to modernize the coding style.

---

## Agent Behavior

When working on this repository:

1. Read this file first.
2. Read `README.md`.
3. Inspect the existing source before changing it.
4. Preserve existing compatibility workarounds.
5. Prefer small incremental changes.
6. Avoid unnecessary rewrites.
7. Keep comments primarily in English.
8. Do not introduce modern frameworks without explicit approval.
9. Do not assume the latest compiler.
10. Treat Windows 2000 compatibility as a real requirement.

If a modern implementation conflicts with Windows 2000 compatibility,
prefer the compatible implementation.

## Design Principle

The project follows one simple rule:

> Make the old machine useful again.

Do not optimize this project for modernity.

Optimize it for:

```text
simplicity
compatibility
understandability
small size
historical curiosity
and actual usefulness
```