#pragma once
#include "wx/wxprec.h"
#include "VulkanCanvas.h"

class VulkanWindow :
    public wxFrame
{
public:
    VulkanWindow(wxWindow* parent, wxWindowID id, const wxString &title);
    virtual ~VulkanWindow();

private:
    void OnResize(wxSizeEvent& event);
    VulkanCanvas* m_canvas;
};

