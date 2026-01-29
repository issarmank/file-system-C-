#ifndef MAINFRAME_H
#define MAINFRAME_H

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <filesystem> // For C++17 filesystem operations

class MainFrame : public wxFrame {
public:
    MainFrame(const wxString& title);

private:
    wxTextCtrl* textBar;
    wxListCtrl* fileList;
    wxButton* openButton;

    std::filesystem::path clipboardSource;
    bool isCutOp;

    void UpdateFileList(const std::filesystem::path& directory);
    void OnOpen(wxListEvent& event);
    void OnCopy(wxCommandEvent& event);
    void OnPaste(wxCommandEvent& event);
    void OnDelete(wxCommandEvent& event);
    void OnRefresh(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnNewFolder(wxCommandEvent& event);
    void OnRename(wxCommandEvent& event);
    void OnCut(wxCommandEvent& event);
    void OnPathEnter(wxCommandEvent& event); // when user types a path and presses Enter

    wxDECLARE_EVENT_TABLE();
};

enum
{
    ID_COPY = wxID_HIGHEST + 1,
    ID_CUT,
    ID_PASTE,
    ID_DELETE,
    ID_REFRESH,
    ID_NEW_FOLDER,
    ID_RENAME,
    ID_EXIT
};

#endif