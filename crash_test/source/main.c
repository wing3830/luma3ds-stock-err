// Crash / ErrDisp test app for the stock-error-handling Luma3DS build.
// Each key triggers one error path, on purpose. Nothing else.
#include <3ds.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    (void)argc; (void)argv;
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);

    printf("Stock error handling test\n");
    printf("=========================\n\n");
    printf("A : ARM11 data abort (null pointer write)\n");
    printf("    stock: black screen / HOME generic error\n");
    printf("    Luma : 'An exception occurred' dump screen\n\n");
    printf("B : svcBreak (USERBREAK_PANIC)\n");
    printf("    stock: process dies, generic handling\n");
    printf("    Luma : 'An exception occurred' (svcBreak)\n\n");
    printf("X : ERRF_Throw (err:f fatal error)\n");
    printf("    stock: Nintendo ErrDisp screen\n");
    printf("    Luma : 'An error occurred (ErrDisp)' + details\n\n");
    printf("START : exit (normal)\n");

    while (aptMainLoop())
    {
        hidScanInput();
        u32 k = hidKeysDown();

        if (k & KEY_START) break;

        if (k & KEY_A)
        {
            printf("\nCrashing now (data abort)...\n");
            gfxFlushBuffers();
            gfxSwapBuffers();
            // Volatile so the compiler cannot optimise the fault away.
            *(volatile u32 *)0x00000000 = 0xDEADBEEF;
            __builtin_unreachable();
        }

        if (k & KEY_B)
        {
            printf("\nBreaking now (svcBreak)...\n");
            gfxFlushBuffers();
            gfxSwapBuffers();
            svcBreak(USERBREAK_PANIC);
            __builtin_unreachable();
        }

        if (k & KEY_X)
        {
            printf("\nThrowing err:f fatal error...\n");
            gfxFlushBuffers();
            gfxSwapBuffers();
            errfInit();
            ERRF_ThrowResultWithMessage(
                MAKERESULT(RL_PERMANENT, RS_INTERNAL, RM_APPLICATION, RD_INVALID_RESULT_VALUE),
                "stock-err test: if you can read this as a plain Nintendo error screen, ErrDisp is stock.");
            __builtin_unreachable();
        }

        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    gfxExit();
    return 0;
}
