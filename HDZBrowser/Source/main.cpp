#define SDL_MAIN_HANDLED
#include "HDZBrowserTool.h"

int main( int argc, char** argv )
{
    ToolCreationFlags flags;
    flags.isBorderless = false;
    flags.isDockingEnabled = true;
    flags.toolName = "HDZ Browser";

    HDZBrowserTool tool( flags );
    tool.Start();

    return 0;
}
