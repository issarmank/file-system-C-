## File Manager (wxWidgets + C++17)

A simple graphical file manager written in C++ using **wxWidgets**. It allows browsing directories and performing common file operations (open, copy/cut/paste, rename, delete, create folder) using a menu, double-click, and an editable path bar.

---

## Features

### File browser UI
- **Directory path bar** (editable). Press **Enter** to navigate to the typed directory.
- **File listing** (wxListCtrl) with columns:
  - Name
  - Type (File/Dir)
  - Size (bytes; blank for directories)
  - Date modified
- **Status bar** for operation feedback.
- Includes a **`..`** entry to navigate to the parent directory.

### File operations (requirements)
- **Open** (double-click):
  - Double-click a directory to enter it.
  - Double-click a file to open it using the system default application.
- **New folder**: prompts for a folder name and creates it in the current directory.
- **Rename**: prompts for a new name for the selected file/folder.
- **Delete**: prompts for confirmation before deleting file/folder.
- **Copy**: stores the selected item path in a *virtual clipboard* (status bar shows message).
- **Cut**: stores the selected item path in a *virtual clipboard* as a move operation (status bar shows message).
- **Paste**: copies/moves the clipboard item into the current directory.
  - Prompts to overwrite if the destination already exists.
  - Clears the clipboard after completion (status bar updated).
- **Refresh**: reloads the current directory listing.
- **Quit**: exits the application (`Ctrl-Q`). On macOS, `wxID_EXIT` may appear under the app menu as well.

---

## Requirements / Dependencies

- macOS (tested with Homebrew wxWidgets)
- C++17 compiler (Apple Clang recommended)
- wxWidgets **3.3.x** (3.2+ should work)

Install wxWidgets via Homebrew:

````bash
brew install wxwidgets
