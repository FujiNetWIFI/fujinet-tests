# Test File Builder

A Windows Forms application for visually creating and editing `.tst` JSON test script files.

## Requirements

- Visual Studio 2022 (17.x)
- .NET 8.0 SDK (Windows)
- `commands.jsn` must be in the same directory as the compiled `.exe`

## Getting Started

1. Open `TestFileBuilder.sln` in Visual Studio 2022
2. Build the solution (Ctrl+Shift+B) — NuGet will restore `Newtonsoft.Json` automatically
3. Run the application (F5 or Ctrl+F5)

> `commands.jsn` is already included in the project and set to copy to the output directory on build.

---

## Usage

### Left Panel — Test List
| Button | Action |
|--------|--------|
| **Add** | Creates a new blank test entry |
| **Remove** | Deletes the selected entry |
| **Copy** | Duplicates the selected entry |
| **▲ / ▼** | Moves the selected entry up or down |

Click any entry in the list to select and edit it in the right panel.

### Right Panel — Editor

#### Command
- **Command** dropdown: lists all commands from `commands.jsn` (sorted A–Z, with command number shown)
- **Device prefix**: optional string added as the `"device"` property (e.g. `apetime`)

#### Flags
- **warnOnly**: adds `"warnOnly": true` to the output
- **errorExpected**: adds `"errorExpected": true` to the output

#### Reply
- **Has Reply**: enable when the command has (or should have) a reply
- **replyLength**: the expected length of the reply data
- **expected**: a literal string or a pattern using:
  - `?` — any single character
  - `#` — a single digit
  - `@` — a single alphabetic character
  - `%` — a single alphanumeric character

#### Arguments
All arguments for the selected command are shown in an editable grid with columns:
- **Argument** — the parameter name
- **Type** — the data type (`bool`, `uint8`, `fixed[256]`, etc.)
- **Value** — the value you enter

Boolean arguments show a dropdown (`true` / `false`).

---

## File Operations

| Menu Item | Shortcut | Description |
|-----------|----------|-------------|
| File > New | Ctrl+N | Start a new (empty) test list |
| File > Open | Ctrl+O | Load an existing `.tst` file |
| File > Save | Ctrl+S | Save to the current file |
| File > Save As | Ctrl+Shift+S | Save to a new file |
| View > JSON Preview | Ctrl+P | Preview the JSON output before saving |
| Help > Pattern Reference | — | Quick reference for `expected` pattern chars |

### Filename Rules
- The output filename must be **8 characters or fewer** (not counting the `.tst` extension)
- The Save dialog will warn you if the name is too long

---

## Output Format

The output is a UTF-8 JSON array. Each test object contains at minimum a `"command"` property. 
Properties appear in this order:
1. `device` (if set)
2. `command`
3. Argument values (matching the command's defined args)
4. `replyLength` (if set)
5. `expected` (if set)
6. `errorExpected` (if set)
7. `warnOnly` (if set)

### Example Output

```json
[
  {
    "command": "set_host_prefix",
    "host_slot": 1,
    "prefix": "/test",
    "warnOnly": true
  },
  {
    "command": "get_host_prefix",
    "host_slot": 1,
    "replyLength": 256,
    "expected": "/test",
    "warnOnly": true
  },
  {
    "device": "apetime",
    "command": "get_time_iso",
    "replyLength": 25
  }
]
```

---

## Argument Types Reference

| Type Char | Meaning | Example |
|-----------|---------|---------|
| `b` | Boolean | `true` / `false` |
| `u` | Unsigned integer | `1`, `64`, `255` |
| `i` | Signed integer | `-1`, `0`, `127` |
| `f` | Fixed-length string/binary | `"/test"` |
| `s` | Variable-length string | `"testing"` |
| `{` | Struct (multiple sub-fields) | Each sub-field shown separately |
