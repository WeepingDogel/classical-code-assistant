# Contributing to Classical Code Assistant

Thank you for contributing to Classical Code Assistant.

This project is experimental and intentionally targets an old operating
system. Contributions should respect the project's compatibility goals.

## Before Contributing

Please understand the main design principle:

> The target is Windows 2000, not a modern Windows desktop.

A solution that works perfectly on Windows 11 but requires a modern runtime
may not be appropriate for this project.

## Development Environment

The preferred development environment is:

- Windows 2000
- Dev-C++
- old MinGW / GCC
- Win32 API

Modern systems may also be used for development and testing.

The source code should remain compatible with the intended legacy toolchain
whenever reasonably possible.

## Coding Style

Keep the code simple.

Prefer:

```cpp
char buffer[256];
````

over introducing a large abstraction for a small task.

Prefer Win32 APIs when they are sufficient.

Avoid unnecessary dependencies.

## C++ Compatibility

The project should avoid relying on modern C++ features unless there is a
clear reason to do so.

In particular, avoid introducing features that cannot be compiled by the
legacy toolchains targeted by this project.

Examples of features that should generally be avoided:

* lambdas
* `auto`
* range-based `for`
* `nullptr`
* C++11/14/17-only libraries
* complex template metaprogramming
* large third-party frameworks

The exact compiler compatibility target may evolve as the project develops.

## Comments

Source-code comments should primarily be written in English.

Prefer comments that explain:

* why something is necessary;
* compatibility limitations;
* non-obvious Win32 behavior;
* protocol details.

Avoid comments that merely repeat the code.

Good:

```cpp
/* Old MinGW may not provide TCITEMA. */
```

Less useful:

```cpp
/* Set the text. */
SetWindowTextA(hEdit, text);
```

## Win32 Compatibility

When adding Windows APIs, consider whether the API exists on Windows 2000.

Do not assume that a function available on Windows 10 or Windows 11 also
exists on Windows 2000.

When possible, check the historical availability of an API before using it.

## GUI Guidelines

The GUI should remain visually compatible with the classic Windows
environment.

Avoid introducing:

* Electron-style interfaces;
* modern web UI;
* unnecessary animations;
* oversized controls;
* modern design-system dependencies.

The goal is a small native application.

## Configuration

Configuration currently uses Windows INI files.

Do not add JSON/XML libraries simply to store a few configuration values.

Use the Win32 profile APIs where appropriate:

```cpp
GetPrivateProfileStringA()
WritePrivateProfileStringA()
```

Configuration files containing API keys must not be committed.

## Dependencies

Before adding a dependency, ask:

1. Is it really necessary?
2. Can Win32 already provide this functionality?
3. Can the Linux gateway handle the complexity instead?
4. Does the dependency work on Windows 2000?
5. Does it work with the old compiler?

A small dependency is preferable to a large framework.

## Network Architecture

The Windows client should not need to directly understand every modern AI
provider.

The preferred architecture is:

```text
Win2000 Client
      |
      | HTTP
      v
Linux AI Gateway
      |
      +---- OpenAI
      |
      +---- DeepSeek
      |
      +---- Ollama
      |
      +---- Other providers
```

Provider-specific logic should generally live in the gateway when practical.

## Pull Requests

A good pull request should contain:

* a clear title;
* a short description;
* the reason for the change;
* compatibility information;
* build/test information.

Example:

```text
Add provider configuration persistence

- Add provider CRUD operations
- Save provider settings to config.ini
- Preserve Windows 2000 compatibility
- Tested with old MinGW
```

## Testing

At minimum, test:

* application startup;
* window resizing;
* tab switching;
* provider creation;
* provider selection;
* provider deletion;
* configuration saving;
* configuration loading;
* invalid configuration handling.

If possible, test on an actual Windows 2000 virtual machine.

## Commit Messages

Keep commit messages short and descriptive.

Examples:

```text
Add provider configuration
Fix old MinGW tab control compatibility
Improve settings layout
Add INI persistence
Add HTTP gateway client
Fix provider deletion
```

Avoid messages such as:

```text
fix
update
stuff
lol
```

## Bug Reports

When reporting a bug, include:

* Windows version;
* compiler/toolchain;
* application version or commit;
* steps to reproduce;
* expected behavior;
* actual behavior;
* compiler/linker errors if applicable.

For example:

```text
OS:
Windows 2000 Datacenter Server

Compiler:
MinGW GCC 3.x

Problem:
Application fails to link.

Error:
undefined reference to `InitCommonControlsEx@4'
```

## Compatibility Bugs

Compatibility bugs are important to this project.

If a feature works on a modern compiler but fails on an old MinGW version,
please report it instead of silently replacing the old toolchain.

Historical compiler behavior is part of the project's development
environment.

## Scope

Classical Code Assistant is not intended to become a general-purpose modern
desktop application.

Features should serve the project's main goals:

* Windows 2000 compatibility;
* native Win32 development;
* lightweight AI interaction;
* simple architecture;
* educational value.

Thank you for helping keep old computers useful.