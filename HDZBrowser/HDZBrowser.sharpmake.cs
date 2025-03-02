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
        conf.VcxprojUserFile.LocalDebuggerWorkingDirectory = Globals.RootDir;

        // Ensure UTF-8 encoding for all builds
        conf.Options.Add(Sharpmake.Options.Vc.Compiler.MultiProcessorCompilation.Enable);
        conf.Options.Add(Sharpmake.Options.Vc.General.CharacterSet.Unicode);  // Use Unicode character set
        conf.Options.Add(Sharpmake.Options.Vc.Compiler.StringPooling.Enable);
    }
}