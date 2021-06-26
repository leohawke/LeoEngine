using Microsoft.VisualStudio.Debugger;

namespace LeoEngine.VSIX
{
    public static class Diagnostic
    {
        public static void NatvisDiagnostic(DkmProcess process,string message)
        {
            var userMessage =DkmUserMessage.Create(process.Connection, process, DkmUserMessageOutputKind.UnfilteredOutputWindowMessage, message, MessageBoxFlags.None, 0);
            userMessage.Post();
        }
    }
}
