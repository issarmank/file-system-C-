#include "MainFrame.h"
#include <wx/aboutdlg.h>
#include <iostream>

// Event Table: This connects your menu/button IDs to specific functions
wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(ID_REFRESH, MainFrame::OnRefresh)
    EVT_MENU(ID_COPY, MainFrame::OnCopy)
    EVT_MENU(ID_PASTE, MainFrame::OnPaste)
    EVT_MENU(wxID_DELETE, MainFrame::OnDelete) // Using standard wxID
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
    menuFile->Append(wxID_EXIT);

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    SetMenuBar(menuBar);

    // 5. Initial population of the list
    UpdateFileList(std::filesystem::current_path());
}