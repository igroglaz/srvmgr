// Original server sets the current directory to the client installation
// directory (by reading "HKLM\SOFTWARE\1C\Allods 2\INSTALLDIR"). We remove
// this odd logic to allow multiple independent servers to be run.
extern "C" void __declspec(naked) overwrite_current_directory() {
    // For some reason, CALL didn't work. So we make a JMP and then jump back.
    __asm {
        mov edx, 0x0048269b
        jmp edx
    }
}
