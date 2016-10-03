#pragma once
#define WIN32_LEAN_AND_MEAN

#include "wx/wxprec.h"
#include <string>

class PaintErrorEvent :
    public wxEvent
{
public:
    PaintErrorEvent(std::string& msg, int id = 0);
    PaintErrorEvent(const PaintErrorEvent&);
    virtual ~PaintErrorEvent();
    inline std::string GetErrorMessage() { return m_errorMessage; }
    wxEvent* Clone() const override;
private:
    std::string m_errorMessage;
};

wxDECLARE_EVENT(EVT_PAINT_EXCEPTION, PaintErrorEvent);