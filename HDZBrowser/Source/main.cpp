#define SDL_MAIN_HANDLED
#include "HDZBrowserTool.h"
#include "CLog.h"

int main( int argc, char** argv )
{
    ToolCreationFlags flags;
    flags.isBorderless = false;
    flags.isDockingEnabled = true;
    flags.toolName = "HDZ Browser";

    CLog::GetInstance().SetLogVerbosity( CLog::LogType::Warning );
    HDZBrowserTool tool( flags );
    tool.Start();

    return 0;
}
