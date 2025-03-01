using Sharpmake;

[Generate]
public class HDZBrowser : BaseTool
{
    public HDZBrowser()
        : base()
    {
        Name = "HDZBrowserTool";
        SourceRootPath = @"Source";
    }

    public override void ConfigureAll(Project.Configuration conf, CommonTarget target)
    {
        base.ConfigureAll(conf, target);
    }
}