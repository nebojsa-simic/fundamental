# Batch Renamer Demo

A simple batch file renamer that demonstrates Fundamental Library's filesystem operations, string manipulation, and console I/O.

## Features

- List files in a directory
- Find-and-replace text in filenames
- Dry-run mode (preview changes)
- Apply mode (actually rename files)

## Build

### Windows
```batch
.\build-windows-amd64.bat
```

### Linux
```bash
./build-linux-amd64.sh
```

## Usage

```batch
demo.exe <directory> <find> <replace> [--apply]
```

### Arguments

| Argument | Description |
|----------|-------------|
| `directory` | Directory containing files to rename |
| `find` | Text to find in filenames |
| `replace` | Text to replace with |
| `--apply` | Optional: actually rename files (default: dry-run) |

### Examples

Preview renaming `.txt` files to `.bak`:
```batch
demo.exe . txt bak
```

Preview replacing "old" with "new" in filenames:
```batch
demo.exe . old new
```

Actually rename files:
```batch
demo.exe . txt bak --apply
```

### Example Output

```
=== Batch Renamer Demo ===
Mode: DRY-RUN (preview only)

Files to rename:

  config.txt -> config.bak
  notes.txt -> notes.bak
  readme.txt -> readme.bak

Summary:
  Files matching pattern: 3
  Run with --apply to rename files
```

## Dependencies

- Filesystem (directory listing, file exists)
- String operations (substring, join, compare)
- Console I/O
- Memory management

## Notes

- Default mode is dry-run for safety
- Use `--apply` flag to actually rename files
- Maximum 8192 bytes for directory listing buffer
