using System.IO;
using Sharpmake;

[module: Sharpmake.Include("Engine/Engine.sharpmake.cs")]
[module: Sharpmake.Include("HDZBrowser/HDZBrowser.sharpmake.cs")]

[Generate]
public class SharpGameProject : BaseGameProject
{
    public SharpGameProject()
        : base()
    {
        Name = "HDZBrowser";
    }
}

[Generate]
public class SharpGameSolution : BaseGameSolution
{
    public SharpGameSolution()
        : base()
    {
        Name = "HDZBrowser";
        ClearTargets();
        Globals.FMOD_Win64_Dir = "C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/";
        Globals.FMOD_macOS_Dir = Util.GetCurrentSharpmakeFileInfo() + "/../ThirdParty/FMOD Programmers API/";
        Globals.MONO_Win64_Dir = string.Empty;
        Globals.IsPhysicsEnabled3D = false;
        Globals.IsUWPEnabled = false;
        Globals.IsUltralightEnabled = false;
        AddTargets(CommonTarget.GetDefaultTargets());
    }

    public override void ConfigureAll(Solution.Configuration conf, CommonTarget target)
    {
        base.ConfigureAll(conf, target);

        conf.AddProject<HDZBrowser>(target);
    }
}

[Generate]
public class UserGameScript : GameScript
{
    public UserGameScript() : base() { }
}

[Generate]
public class UserSharpmakeProject : SharpmakeProjectBase
{
    public UserSharpmakeProject() : base()
    {
        SourceFiles.Add(@"HDZBrowser/HDZBrowser.sharpmake.cs");
    }
}