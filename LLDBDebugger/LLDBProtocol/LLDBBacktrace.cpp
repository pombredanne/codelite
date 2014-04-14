#include "LLDBBacktrace.h"
#include <wx/filename.h>

LLDBBacktrace::LLDBBacktrace(lldb::SBThread &thread)
{
    m_callstack.clear();
    if ( thread.IsValid() ) {
        m_threadId = thread.GetIndexID();
        int nFrames = thread.GetNumFrames();
        for(int j=0; j<nFrames; ++j) {
            
            lldb::SBFrame frame = thread.GetFrameAtIndex(j);
            LLDBBacktrace::Entry entry;
            
            if ( frame.IsValid() ) {
                // do we have a file:line?
                if ( frame.GetLineEntry().IsValid() ) {

                    lldb::SBFileSpec fileSepc = frame.GetLineEntry().GetFileSpec();
                    entry.filename      = wxFileName(fileSepc.GetDirectory(), fileSepc.GetFilename()).GetFullPath();
                    entry.functionName  = frame.GetFunctionName();
                    entry.line          = frame.GetLineEntry().GetLine()-1;
                    entry.id            = j;
                    entry.address << wxString::Format("%p", (void*)frame.GetFP());
                    m_callstack.push_back( entry );

                } else {
                    // FIXME: if we dont have a debug symbol, we should learn how to construct a proper entry
                    // for now, we add an empty entry
                    entry.functionName = "??";
                    entry.id = j;
                    m_callstack.push_back( entry );
                }

            }
        }
    }
}

LLDBBacktrace::~LLDBBacktrace()
{
}

wxString LLDBBacktrace::ToString() const
{
    wxString str;
    str << "Thread ID: " << m_threadId << "\n";
    for(size_t i=0; i<m_callstack.size(); ++i) {
        const LLDBBacktrace::Entry& entry = m_callstack.at(i);
        str << entry.address << ", " 
            << entry.functionName << ", " 
            << entry.filename << ", " 
            << entry.line << "\n";
    }
    return str;
}

void LLDBBacktrace::FromJSON(const JSONElement& json)
{
    m_callstack.clear();
    m_threadId = json.namedObject("m_threadId").toInt(0);
    JSONElement arr = json.namedObject("m_callstack");
    for(int i=0; i<arr.arraySize(); ++i) {
        LLDBBacktrace::Entry entry;
        entry.FromJSON( arr.arrayItem(i) );
        m_callstack.push_back( entry );
    }
}

JSONElement LLDBBacktrace::ToJSON() const
{
    JSONElement json = JSONElement::createObject();
    json.addProperty("m_threadId", m_threadId);
    JSONElement arr = JSONElement::createArray("m_callstack");
    json.append( arr );
    
    for(size_t i=0; i<m_callstack.size(); ++i) {
        arr.append( m_callstack.at(i).ToJSON() );
    }
    return json;
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

void LLDBBacktrace::Entry::FromJSON(const JSONElement& json)
{
    id = json.namedObject("id").toInt(0);
    line = json.namedObject("line").toInt(0);
    filename = json.namedObject("filename").toString();
    functionName = json.namedObject("functionName").toString();
    address = json.namedObject("address").toString();
}

JSONElement LLDBBacktrace::Entry::ToJSON() const
{
    JSONElement json = JSONElement::createObject();
    json.addProperty("id", id);
    json.addProperty("line", line);
    json.addProperty("filename", filename);
    json.addProperty("functionName", functionName);
    json.addProperty("address", address);
    return json;
}