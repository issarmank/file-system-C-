#include <wx/wx.h>
#include "MainFrame.h"

// Define the Application class
class MyApp : public wxApp {
public:
    // This function runs when the program starts
    virtual bool OnInit() {
        // Create the main window defined in MainFrame.h
        MainFrame *frame = new MainFrame("File Manager");
        
        // Show the window
        frame->Show(true);
        
        // Return true to start the event loop
        return true;
    }
};

// This macro creates the main() function and starts everything
wxIMPLEMENT_APP(MyApp);