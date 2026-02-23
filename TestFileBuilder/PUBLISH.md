# Publishing as Single-File Executable

To create a single-file executable with only the .exe and commands.jsn:

## Option 1: Visual Studio

1. Right-click the **TestFileBuilder** project in Solution Explorer
2. Select **Publish...**
3. Choose **Folder** as the target
4. Click **Show all settings**
5. Verify settings:
   - **Target Runtime**: win-x64
   - **Deployment Mode**: Self-contained
   - **Produce single file**: Checked
6. Click **Publish**
7. Find the output in `bin\Release\net8.0-windows\win-x64\publish\`

## Option 2: Command Line

Open a terminal in the solution directory and run:

```bash
dotnet publish TestFileBuilder/TestFileBuilder.csproj -c Release -r win-x64 --self-contained true -p:PublishSingleFile=true
```

Output will be in:
```
TestFileBuilder/bin/Release/net8.0-windows/win-x64/publish/
```

## What Gets Published

The publish process will automatically create:
- **TestJSONBuilder.zip** - Ready-to-distribute archive containing:
  - TestFileBuilder.exe (single-file executable, ~70-90 MB - includes .NET runtime)
  - commands.jsn (required configuration file)

The .pdb debug file is NOT included in the publish output.

The fujinet.ico is embedded in the .exe and doesn't need to be distributed separately.

## Distribution

Simply distribute the **TestJSONBuilder.zip** file. Users extract it and run TestFileBuilder.exe.

Both files must remain in the same directory:
1. TestFileBuilder.exe
2. commands.jsn

Users do NOT need .NET installed - the runtime is bundled in the executable.

## Notes

- The first run may be slightly slower as the runtime extracts itself
- The executable is platform-specific (this build is for Windows x64)
- To build for other platforms, change the RuntimeIdentifier:
  - `win-x64` - Windows 64-bit
  - `win-x86` - Windows 32-bit
  - `win-arm64` - Windows ARM64
