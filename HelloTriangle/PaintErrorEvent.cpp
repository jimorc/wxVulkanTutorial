#include "PaintErrorEvent.h"

wxDEFINE_EVENT(EVT_PAINT_EXCEPTION, PaintErrorEvent);

PaintErrorEvent::PaintErrorEvent(std::string& msg, int id)
    : wxEvent(id, EVT_PAINT_EXCEPTION), m_errorMessage(msg)
{
}

PaintErrorEvent::PaintErrorEvent(const PaintErrorEvent& rhs)
    : wxEvent(rhs), m_errorMessage(rhs.m_errorMessage)
{
}

/*PaintErrorEvent& PaintErrorEvent::operator=(const PaintErrorEvent& rhs)
{
    PaintErrorEvent event(rhs);
    return event;
}*/

PaintErrorEvent::~PaintErrorEvent()
{
}

wxEvent * PaintErrorEvent::Clone() const
{
    PaintErrorEvent* event = new PaintErrorEvent(*this);
    return event;
}
