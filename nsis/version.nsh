; the single source of truth for the version is src/version.h:
; this file derives everything from it at compile time, so bumping the
; version means editing exactly one file (plus Changes.txt).
!searchparse /noerrors /file "..\src\version.h" `#define VERSION_MAJOR ` VIV_VER_MAJOR
!searchparse /noerrors /file "..\src\version.h" `#define VERSION_MINOR ` VIV_VER_MINOR
!searchparse /noerrors /file "..\src\version.h" `#define VERSION_REVISION ` VIV_VER_REVISION
!searchparse /noerrors /file "..\src\version.h" `#define VERSION_BUILD ` VIV_VER_BUILD
!searchparse /noerrors /file "..\src\version.h" `#define VERSION_YEAR ` VIV_VER_YEAR
!searchparse /noerrors /file "..\src\version.h" `#define VERSION_TYPE "` VIV_VER_TYPE `"`
!searchparse /noerrors /file "..\src\version.h" `#define VERSION_STRING "` VIV_VER_STRING `"`

!define VERSION "${VIV_VER_MAJOR}.${VIV_VER_MINOR}.${VIV_VER_REVISION}.${VIV_VER_BUILD}"
!define BETAVERSION "${VIV_VER_TYPE}"
!define DISPLAYVERSION "${VIV_VER_STRING}"
!define VERSIONYEAR "${VIV_VER_YEAR}"
