// voidImageViewer -- self screenshot harness (test builds only)
//
// compiled out completely unless VIVP_SELF_SHOT is defined at build time.
#ifndef _VIV_SELFSHOT_H
#define _VIV_SELFSHOT_H
#ifdef VIVP_SELF_SHOT
// read VIVP_SELF_SHOT from the environment and arm the capture timer.
// returns 1 if the harness is active.
int vivp_selfshot_init(void);
#endif // VIVP_SELF_SHOT
#endif // _VIV_SELFSHOT_H
