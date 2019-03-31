#pragma once
#include <wx/wx.h>

class wxVulkanTutorialApp :
    public wxApp
{
public:
    wxVulkanTutorialApp();
    virtual ~wxVulkanTutorialApp();
    virtual bool OnInit() override;
};

