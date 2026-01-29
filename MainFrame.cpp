#include "MainFrame.h"
#include <wx/aboutdlg.h>
#include <wx/utils.h>      // wxLaunchDefaultApplication
#include <wx/filename.h>   // wxFileName
#include <wx/msgdlg.h>     // wxMessageBox
#include <wx/textdlg.h>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>

// Event Table: This connects your menu/button IDs to specific functions
wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(ID_REFRESH, MainFrame::OnRefresh)
    EVT_MENU(ID_COPY, MainFrame::OnCopy)
    EVT_MENU(ID_PASTE, MainFrame::OnPaste)
    EVT_MENU(wxID_DELETE, MainFrame::OnDelete) // Using standard wxID
    EVT_MENU(wxID_EXIT, MainFrame::OnExit)
    EVT_MENU(ID_EXIT, MainFrame::OnExit)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, MainFrame::OnOpen) // Double-click
wxEND_EVENT_TABLE()

MainFrame::MainFrame(const wxString& title)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(800, 600)) {

    // 1. Initialize the Status Bar for feedback
    CreateStatusBar();
    SetStatusText("Welcome to the File Manager!");

    // 2. Create UI Components
    wxPanel* panel = new wxPanel(this);
    textBar = new wxTextCtrl(panel, wxID_ANY, wxGetCwd()); // Start at current path
    fileList = new wxListCtrl(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);

    // Add columns to the file list as required
    fileList->InsertColumn(0, "Name", wxLIST_FORMAT_LEFT, 300);
    fileList->InsertColumn(1, "Type", wxLIST_FORMAT_LEFT, 100);
    fileList->InsertColumn(2, "Size", wxLIST_FORMAT_LEFT, 100);
    fileList->InsertColumn(3, "Date", wxLIST_FORMAT_LEFT, 200);

    // 3. Setup the Layout (Sizers)
    // This stacks the textBar and fileList vertically
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(textBar, 0, wxEXPAND | wxALL, 5);
    mainSizer->Add(fileList, 1, wxEXPAND | wxALL, 5);
    panel->SetSizer(mainSizer);

    // 4. Create the Menu Bar
    wxMenu* menuFile = new wxMenu;
    menuFile->Append(ID_REFRESH, "&Refresh\tCtrl-R");
    menuFile->AppendSeparator();
    menuFile->Append(ID_COPY, "&Copy\tCtrl-C");
    menuFile->Append(ID_PASTE, "&Paste\tCtrl-V");
    menuFile->Append(wxID_DELETE, "&Delete\tDel");
    menuFile->AppendSeparator();
    menuFile->Append(ID_EXIT, "&Quit\tCtrl-Q");
    menuFile->Append(wxID_EXIT, "Quit &FileManager\tCtrl-Q");


    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    SetMenuBar(menuBar);

    // 5. Initial population of the list
    UpdateFileList(std::filesystem::current_path());
}

static wxString FormatTime(std::filesystem::file_time_type ft)
{
    // Convert filesystem clock -> system_clock (C++17-friendly approach)
    using namespace std::chrono;
    const auto sctp = time_point_cast<system_clock::duration>(
        ft - std::filesystem::file_time_type::clock::now() + system_clock::now());

    std::time_t tt = system_clock::to_time_t(sctp);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M");
    return wxString(oss.str());
}

void MainFrame::UpdateFileList(const std::filesystem::path& directory)
{
    fileList->DeleteAllItems();

    std::error_code ec;
    if (!std::filesystem::exists(directory, ec) || !std::filesystem::is_directory(directory, ec))
    {
        wxMessageBox("Path is not a directory.", "Error", wxOK | wxICON_ERROR, this);
        return;
    }

    // Optional ".." entry to go up (skip if already the root)
    if (directory.has_parent_path())
    {
        long i = fileList->InsertItem(fileList->GetItemCount(), "..");
        fileList->SetItem(i, 1, "Dir");
        fileList->SetItem(i, 2, "");
        fileList->SetItem(i, 3, "");
    }

    for (const auto& entry : std::filesystem::directory_iterator(directory, ec))
    {
        if (ec) break;

        const auto p = entry.path();
        const auto name = p.filename().string();

        long row = fileList->InsertItem(fileList->GetItemCount(), name);

        const bool isDir = entry.is_directory(ec);
        fileList->SetItem(row, 1, isDir ? "Dir" : "File");

        if (!isDir)
        {
            auto sz = entry.file_size(ec);
            fileList->SetItem(row, 2, ec ? "" : std::to_string(static_cast<unsigned long long>(sz)));
        }
        else
        {
            fileList->SetItem(row, 2, "");
        }

        auto mod = entry.last_write_time(ec);
        fileList->SetItem(row, 3, ec ? "" : FormatTime(mod));
    }
}

static bool GetSelectedName(wxListCtrl* list, wxString& outName)
{
    long sel = list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (sel == -1) return false;
    outName = list->GetItemText(sel, 0);
    return !outName.IsEmpty();
}

void MainFrame::OnOpen(wxListEvent& event)
{
    try
    {
        const long idx = event.GetIndex();
        if (idx < 0) return;

        // Current directory shown in the text bar
        const wxString cwdWx = textBar->GetValue();
        if (cwdWx.IsEmpty()) return;

        const std::filesystem::path cwd = std::filesystem::path(std::string(cwdWx.mb_str()));
        if (!std::filesystem::exists(cwd) || !std::filesystem::is_directory(cwd))
        {
            wxMessageBox("Current path is not a directory.", "Error", wxOK | wxICON_ERROR, this);
            return;
        }

        // Get item name from first column ("Name")
        wxString nameWx = fileList->GetItemText(idx, 0);

        // Handle ".." manually (if you add it to the list)
        if (nameWx == "..")
        {
            std::filesystem::path parent = cwd.parent_path();
            if (parent.empty()) parent = cwd; // stay put if no parent
            textBar->SetValue(parent.string());
            UpdateFileList(parent);
            SetStatusText("Moved to parent directory.");
            return;
        }

        const std::filesystem::path target = cwd / std::string(nameWx.mb_str());

        if (!std::filesystem::exists(target))
        {
            wxMessageBox("Selected item no longer exists.", "Error", wxOK | wxICON_ERROR, this);
            UpdateFileList(cwd);
            return;
        }

        if (std::filesystem::is_directory(target))
        {
            // Navigate into directory
            textBar->SetValue(target.string());
            UpdateFileList(target);
            SetStatusText("Opened directory: " + target.filename().string());
            return;
        }

        // Launch file with default application
        const wxString targetWx = wxString::FromUTF8(target.string());
        if (!wxLaunchDefaultApplication(targetWx))
        {
            wxMessageBox("Could not open file with default application.", "Open failed",
                         wxOK | wxICON_ERROR, this);
            return;
        }

        SetStatusText("Opened file: " + target.filename().string());
    }
    catch (const std::exception& ex)
    {
        wxMessageBox(wxString::Format("Open failed: %s", ex.what()),
                     "Error", wxOK | wxICON_ERROR, this);
    }
}

void MainFrame::OnCopy(wxCommandEvent& event)
{
    wxString name;
    if (!GetSelectedName(fileList, name))
    {
        wxMessageBox("Select a file or folder first.", "Copy", wxOK | wxICON_INFORMATION, this);
        return;
    }

    const std::filesystem::path cwd = std::filesystem::path(std::string(textBar->GetValue().mb_str()));
    clipboardSource = cwd / std::string(name.mb_str());
    isCutOp = false;

    SetStatusText("Copied to clipboard: " + clipboardSource.filename().string());
}

void MainFrame::OnPaste(wxCommandEvent& event)
{
    if (clipboardSource.empty())
    {
        wxMessageBox("Clipboard is empty.", "Paste", wxOK | wxICON_INFORMATION, this);
        return;
    }

    std::error_code ec;
    const std::filesystem::path destDir = std::filesystem::path(std::string(textBar->GetValue().mb_str()));
    const std::filesystem::path destPath = destDir / clipboardSource.filename();

    if (std::filesystem::exists(destPath, ec))
    {
        int ans = wxMessageBox("Item already exists. Overwrite?", "Confirm overwrite",
                               wxYES_NO | wxICON_WARNING, this);
        if (ans != wxYES) return;

        std::filesystem::remove_all(destPath, ec); // remove file/dir to overwrite
        if (ec)
        {
            wxMessageBox("Could not remove existing item.", "Error", wxOK | wxICON_ERROR, this);
            return;
        }
    }

    // Copy or move
    if (isCutOp)
    {
        std::filesystem::rename(clipboardSource, destPath, ec);
        if (ec)
        {
            // cross-device rename can fail; fallback to copy+remove
            ec.clear();
            if (std::filesystem::is_directory(clipboardSource, ec))
                std::filesystem::copy(clipboardSource, destPath, std::filesystem::copy_options::recursive, ec);
            else
                std::filesystem::copy_file(clipboardSource, destPath, std::filesystem::copy_options::overwrite_existing, ec);

            if (ec)
            {
                wxMessageBox("Move failed.", "Error", wxOK | wxICON_ERROR, this);
                return;
            }

            std::filesystem::remove_all(clipboardSource, ec);
            if (ec)
            {
                wxMessageBox("Move partially failed (could not delete source).", "Warning",
                             wxOK | wxICON_WARNING, this);
            }
        }
    }
    else
    {
        if (std::filesystem::is_directory(clipboardSource, ec))
            std::filesystem::copy(clipboardSource, destPath, std::filesystem::copy_options::recursive, ec);
        else
            std::filesystem::copy_file(clipboardSource, destPath, std::filesystem::copy_options::overwrite_existing, ec);

        if (ec)
        {
            wxMessageBox("Copy failed.", "Error", wxOK | wxICON_ERROR, this);
            return;
        }
    }

    clipboardSource.clear();
    isCutOp = false;
    SetStatusText("Paste complete. Clipboard empty.");

    UpdateFileList(destDir);
}

void MainFrame::OnDelete(wxCommandEvent& event)
{
    wxString name;
    if (!GetSelectedName(fileList, name))
    {
        wxMessageBox("Select a file or folder first.", "Delete", wxOK | wxICON_INFORMATION, this);
        return;
    }

    const std::filesystem::path cwd = std::filesystem::path(std::string(textBar->GetValue().mb_str()));
    const std::filesystem::path target = cwd / std::string(name.mb_str());

    int ans = wxMessageBox("Delete selected item?\n" + wxString::FromUTF8(target.string()),
                           "Confirm delete", wxYES_NO | wxICON_WARNING, this);
    if (ans != wxYES) return;

    std::error_code ec;
    std::filesystem::remove_all(target, ec);
    if (ec)
    {
        wxMessageBox("Delete failed.", "Error", wxOK | wxICON_ERROR, this);
        return;
    }

    SetStatusText("Deleted: " + target.filename().string());
    UpdateFileList(cwd);
}

void MainFrame::OnRefresh(wxCommandEvent& event)
{
    const std::filesystem::path cwd = std::filesystem::path(std::string(textBar->GetValue().mb_str()));
    UpdateFileList(cwd);
    SetStatusText("Refreshed.");
}

void MainFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}