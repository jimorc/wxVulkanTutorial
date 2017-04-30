#include <wx/wxprec.h>
#include <sstream>
#include "wxVulkanTutorialApp.h"
#include "VulkanWindow.h"
#include "VulkanException.h"

#pragma warning(disable: 28251)

#ifdef _UNICODE
#ifdef _DEBUG
#pragma comment(lib, "wxbase31ud.lib")
#else
#pragma comment(lib, "wxbase31u.lib")
#endif
#else
#ifdef _DEBUG
#pragma comment(lib, "wxbase31d.lib")
#else
#pragma comment(lib, "wxbase31.lib")
#endif
#endif

wxVulkanTutorialApp::wxVulkanTutorialApp()
{
}

wxVulkanTutorialApp::~wxVulkanTutorialApp()
{
}

bool wxVulkanTutorialApp::OnInit()
{
    VulkanWindow* mainFrame;
    try {
        mainFrame = new VulkanWindow(nullptr, wxID_ANY, L"UniformBuffersApp");
    } 
    catch(VulkanException& ve) {
        std::string status = ve.GetStatus();
        std::stringstream ss;
        ss << ve.what() << "\n" << status;
        wxMessageBox(ss.str(), "Vulkan Error");
        return false;
    }
    catch (std::runtime_error& err) {
        std::stringstream ss;
        ss << "Error encountered trying to create the Vulkan canvas:\n";
        ss << err.what();
        wxMessageBox(ss.str(), "Vulkan Error");
        return false;
    }
    mainFrame->Show(true);
    return true;
}

wxIMPLEMENT_APP(wxVulkanTutorialApp);