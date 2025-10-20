#Requires AutoHotkey v2.0

^+z:: ; Ctrl+Shift+Z
{
    exe_path  := "C:\Program Files (x86)\cappy\bin\cappy.exe"
    exe_title := "Cappy"

    ; If already running, treat shortcut as minimize/maximize.
    if (PID := WinExist(exe_title)) {
        if (WinActive("ahk_id " PID)) {
            WinMinimize("ahk_id " PID)
        } else {
            WinActivate("ahk_id " PID)
            WinRestore("ahk_id " PID)
        }
        return
    }

    ; Otherwise try to run the program.
    try {
        Run(exe_path, , ,)
    } catch {
        MsgBox Format("'{}' failed to run.", exe_path)
    }

    return
}
