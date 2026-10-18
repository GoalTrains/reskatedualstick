using System;
using System.Diagnostics;
using System.IO;
using System.Text;
using System.Windows.Forms;

internal static class DualStickLauncher
{
    internal static string Quote(string value)
    {
        var result = new StringBuilder("\"");
        int slashes = 0;
        foreach (char c in value)
        {
            if (c == '\\') { slashes++; continue; }
            result.Append('\\', c == '"' ? slashes * 2 + 1 : slashes);
            result.Append(c); slashes = 0;
        }
        result.Append('\\', slashes * 2); result.Append('"');
        return result.ToString();
    }

    private static void Log(string root, string message)
    {
        try { File.AppendAllText(Path.Combine(root, "dual_stick_updater.log"), DateTime.UtcNow.ToString("u") + " " + message + Environment.NewLine); }
        catch { }
    }

    [STAThread]
    private static int Main(string[] args)
    {
        string root = AppDomain.CurrentDomain.BaseDirectory;
        string launcher = Path.Combine(root, "ReSkateLauncher.exe");
        if (!File.Exists(launcher))
        {
            MessageBox.Show("Put DualStickLauncher.exe beside ReSkateLauncher.exe in your Skate folder.", "Dual Stick updater", MessageBoxButtons.OK, MessageBoxIcon.Error);
            return 1;
        }
        try
        {
            string script = Path.Combine(root, "updater", "Update-DualStick.ps1");
            if (File.Exists(script))
            {
                string powershell = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.Windows), "System32", "WindowsPowerShell", "v1.0", "powershell.exe");
                var update = new ProcessStartInfo(powershell,
                    "-STA -NoProfile -NonInteractive -ExecutionPolicy Bypass -File " + Quote(script) + " -GameDirectory " + Quote(root) + " -Interactive");
                update.WorkingDirectory = root;
                update.UseShellExecute = false;
                update.CreateNoWindow = true;
                update.WindowStyle = ProcessWindowStyle.Hidden;
                using (var process = Process.Start(update))
                {
                    // Network operations have their own timeouts. User dialogs
                    // must remain open until the user decides; never time out consent.
                    process.WaitForExit();
                    Log(root, "Updater process finished with exit code " + process.ExitCode + ".");
                }
            }
            else { Log(root, "Updater script missing; opening the official launcher."); }
        }
        catch (Exception error) { Log(root, "Update skipped: " + error.Message); }

        try
        {
            var arguments = new StringBuilder();
            foreach (string argument in args)
            {
                if (arguments.Length > 0) arguments.Append(' ');
                arguments.Append(Quote(argument));
            }
            using (var process = Process.Start(new ProcessStartInfo(launcher, arguments.ToString()) { WorkingDirectory = root, UseShellExecute = false }))
            {
                Log(root, "Official launcher process started (PID " + process.Id + ").");
            }
            return 0;
        }
        catch (Exception error)
        {
            MessageBox.Show("Could not open the official ReSkate launcher: " + error.Message, "Dual Stick updater", MessageBoxButtons.OK, MessageBoxIcon.Error);
            return 1;
        }
    }
}
