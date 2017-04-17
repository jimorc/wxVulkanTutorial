#pragma once
#include <wx/wxprec.h>

class wxVulkanTutorialApp :
    public wxApp
{
public:
    wxVulkanTutorialApp();
    virtual ~wxVulkanTutorialApp();
    virtual bool OnInit() override;
};

