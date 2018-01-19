using System.Runtime.InteropServices;

public static class ModuleInitializer
{
    [DllImport("leo.Engine.Net")]
    static extern void leo_Net_CoreCLRInject();


    public static void Initialize()
    {
        leo_Net_CoreCLRInject();
    }
}
