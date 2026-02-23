using TestFileBuilder;

[assembly: System.Runtime.Versioning.SupportedOSPlatform("windows")]

namespace TestFileBuilder
{
    internal static class Program
    {
        [STAThread]
        static void Main()
        {
            ApplicationConfiguration.Initialize();
            Application.Run(new MainForm());
        }
    }
}
