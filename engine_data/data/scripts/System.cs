using System;
using System.Runtime.CompilerServices;

namespace Ace
{
namespace Core
{

public static class SystemManager
{
    public static event Action OnUpdate;

    public static void Update()
    {
        OnUpdate?.Invoke();
    }
}

public abstract class ISystem
{
    public ISystem()
    {
        SystemManager.OnUpdate += Update;
    }

    ~ISystem()
    {
        SystemManager.OnUpdate -= Update;
    }

    public abstract void Update();
}

public static class Logger
{
    public static void LogTrace(string message,
                                [CallerMemberName] string func = "",
                                [CallerFilePath] string file = "",
                                [CallerLineNumber] int line = 0)
    {
        Internal_LogTrace(message, func, file, line);
    }
    public static void LogInfo(string message,
                               [CallerMemberName] string func = "",
                               [CallerFilePath] string file = "",
                               [CallerLineNumber] int line = 0)
    {
        Internal_LogInfo(message, func, file, line);
    }
    public static void LogWarning(string message,
                                  [CallerMemberName] string func = "",
                                  [CallerFilePath] string file = "",
                                  [CallerLineNumber] int line = 0)
    {
        Internal_LogWarning(message, func, file, line);
    }
    public static void LogError(string message,
                                [CallerMemberName] string func = "",
                                [CallerFilePath] string file = "",
                                [CallerLineNumber] int line = 0)
    {
        Internal_LogError(message, func, file, line);
    }


    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void Internal_LogTrace(string message, string func, string file, int line);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void Internal_LogInfo(string message, string func, string file, int line);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void Internal_LogWarning(string message, string func, string file, int line);

    [MethodImpl(MethodImplOptions.InternalCall)]
    private static extern void Internal_LogError(string message, string func, string file, int line);

}



}

}


