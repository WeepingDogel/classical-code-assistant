# Classical Code Assistant

> A native Win32 AI client for Windows 2000.

Classical Code Assistant is an experimental AI client designed to run on
legacy Windows systems, especially Windows 2000.

The project intentionally avoids modern frameworks such as Electron, Qt,
.NET, and other heavy runtimes. The goal is to build a small, native,
classic Win32 application that can still communicate with modern AI services
through a separate gateway.

The project is also an experiment in developing software under historical
technical constraints.

## Project Goals

- Run natively on Windows 2000.
- Use the Win32 API directly.
- Build with old versions of MinGW / Dev-C++.
- Avoid large third-party GUI frameworks.
- Keep the client lightweight.
- Support multiple AI provider configurations.
- Store configuration in a simple INI file.
- Use a modern Linux server as an AI gateway.
- Keep the architecture simple enough to understand and maintain.

## Architecture

```text
┌──────────────────────────────┐
│       Windows 2000           │
│                              │
│  Classical Code Assistant    │
│                              │
│  Native Win32 GUI            │
│  Old MinGW / Dev-C++         │
└──────────────┬───────────────┘
               │
               │ HTTP
               │
               ▼
┌──────────────────────────────┐
│       Linux AI Gateway       │
│                              │
│  FastAPI / Python            │
│                              │
│  API compatibility layer     │
└──────────────┬───────────────┘
               │
       ┌───────┼────────┐
       ▼       ▼        ▼
    OpenAI  DeepSeek  Ollama
````

The Windows 2000 client is intentionally kept simple.

Modern TLS, authentication, provider-specific APIs, and other complicated
network operations can be handled by the Linux gateway instead of the
legacy operating system.

## Why Windows 2000?

Modern software development environments increasingly assume:

* modern operating systems;
* modern TLS implementations;
* recent C++ standards;
* large runtime environments;
* package managers;
* modern browsers;
* Git and other recent developer tools.

This project explores the opposite direction:

> How much useful software can we still build when the target system is
> Windows 2000?

The limitations are part of the project.

## Features

Current / planned features include:

* Native Win32 GUI
* Windows 2000 compatible interface
* Chat interface
* Provider configuration
* Multiple provider profiles
* Provider selection
* API endpoint configuration
* API key configuration
* Model configuration
* System prompt configuration
* INI-based configuration
* Persistent per-provider chat history
* Classic Windows-style progress indicator
* No external GUI framework

## Current Status

This project is currently in early development.

The GUI and configuration system are being implemented first.

The AI network layer will be added later.

At the current stage, the application may use a simulated response instead
of communicating with an actual AI provider.

## Project Structure

```text
classical-code-assistant/
│
├── src/
│   ├── main.cpp
│   ├── gui.cpp
│   ├── gui.h
│   ├── config.cpp
│   ├── config.h
│   ├── history.cpp
│   └── history.h
│
├── README.md
├── CONTRIBUTION.md
├── AGENTS.md
└── .gitignore
```

## Configuration

Configuration is stored in an INI file next to the executable.

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

[Provider1]
Name=DeepSeek Gateway
Endpoint=http://192.168.1.100:8000/v1/chat
ApiKey=
Model=deepseek-chat
SystemPrompt=You are a helpful AI assistant.
```

The configuration file should not be committed to Git because it may contain
API keys.

## Chat History

Chat history is stored as plain-text files next to the executable, in a
`chatlog\` directory:

```text
chatlog\
├── Provider0.txt
├── Provider1.txt
└── ...
```

Each file holds timestamped user/AI exchanges, one per provider.  The files
are opened with Notepad or any text editor on Windows 2000.

* The **View History** button on the Chat tab opens the `chatlog\` folder in
  a file picker, and loads the chosen log into the response box.
* The active provider's log is also auto-loaded when the app starts, so the
  last conversation is visible right away.
* When a new response arrives, both the user's message and the AI's reply are
  appended to the active provider's log.

## Building

The project is intended to be buildable with old MinGW toolchains.

A modern compiler is not required for the core project.

### Command line

Example:

```bash
g++ main.cpp gui.cpp config.cpp history.cpp -o classical-code-assistant.exe -mwindows -lcomctl32 -lcomdlg32
```

Or, from the project root:

```bash
g++ src/main.cpp src/gui.cpp src/config.cpp src/history.cpp \
    -o classical-code-assistant.exe \
    -mwindows \
    -lcomctl32 \
    -lcomdlg32
```

### Important: `-lcomctl32`

`-lcomctl32` is a linker option.

Do not use it with:

```bash
g++ -c
```

For example, this is incorrect:

```bash
g++ -c gui.cpp -lcomctl32
```

Compile first:

```bash
g++ -c gui.cpp
```

Then link:

```bash
g++ main.o gui.o config.o history.o -o classical-code-assistant.exe -mwindows -lcomctl32 -lcomdlg32
```

In old Dev-C++, put:

```text
-lcomctl32 -lcomdlg32
```

in the linker parameters rather than the compile-only parameters.

`-lcomdlg32` is required for the chat history file picker
(`GetOpenFileNameA`).

## Windows 2000 Compatibility

The project deliberately avoids relying on modern Windows APIs.

The code should prefer:

* Win32 API
* ANSI APIs where necessary
* old-compatible C/C++ syntax
* simple data structures
* standard Windows controls
* INI configuration APIs

The project also avoids depending on:

* .NET
* Electron
* Qt
* Chromium
* modern C++ standard library features
* modern Windows-only APIs

## Networking

Direct communication from Windows 2000 to modern HTTPS services is
problematic because of obsolete TLS and certificate support.

Therefore the preferred architecture is:

```text
Windows 2000
      │
      │ HTTP
      ▼
Linux Gateway
      │
      │ HTTPS / provider API
      ▼
AI Provider
```

The gateway should handle modern HTTPS connections.

For security, the HTTP connection between Windows 2000 and the gateway should
normally be restricted to a trusted local network or protected by an
appropriate network security layer.

Do not expose an unauthenticated AI gateway directly to the public Internet.

## Development Philosophy

This project is intentionally small.

We prefer:

```text
simple code
    >
clever code
```

and:

```text
understandable dependencies
    >
large frameworks
```

The application should remain understandable even when viewed years later.

## Roadmap

### Phase 1 — Native GUI

* [x] Win32 application
* [x] Chat interface
* [x] Settings interface
* [x] Provider selection
* [x] Provider configuration
* [x] INI configuration
* [x] Persistent chat history

### Phase 2 — Network Layer

* [ ] HTTP client
* [ ] Linux gateway integration
* [ ] Request serialization
* [ ] Response parsing
* [ ] Error handling
* [ ] Connection timeout

### Phase 3 — AI Gateway

* [ ] FastAPI gateway
* [ ] OpenAI-compatible interface
* [ ] DeepSeek support
* [ ] Ollama support
* [ ] Provider routing
* [ ] Authentication

### Phase 4 — Usability

* [ ] Chat history
* [ ] Copy response
* [ ] Clear conversation
* [ ] Logging
* [ ] Better error messages
* [ ] Keyboard shortcuts
* [ ] Classic Windows icon

### Phase 5 — Retro Compatibility

* [ ] Test on Windows 2000 Professional
* [ ] Test on Windows 2000 Server
* [ ] Test on Windows 2000 Datacenter Server
* [ ] Test on old physical hardware
* [ ] Document supported compiler versions

## License

License: TBD.