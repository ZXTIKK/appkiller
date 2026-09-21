//
// Created by Саша on 20.09.2026.
//

#ifndef APPKILLER_VERSION_H
#define APPKILLER_VERSION_H

// ============================================================================
#define VER_MAJOR               1
#define VER_MINOR               1
#define VER_PATCH               1
#define VER_BUILD               0

#define VER_COMPANY_NAME_STR    "zxnt"
#define VER_PRODUCTNAME_STR     "AppKiller"
#define VER_FILEDESCRIPTION_STR "AppKiller Utility"
// ============================================================================

#define _STR(x) #x
#define STR(x) _STR(x)

#define VER_NUMERIC             VER_MAJOR,VER_MINOR,VER_PATCH,VER_BUILD

#define VER_FULL_STRING         STR(VER_MAJOR) "." STR(VER_MINOR) "." STR(VER_PATCH) "." STR(VER_BUILD)

#define VER_STRING              STR(VER_MAJOR) "." STR(VER_MINOR) "." STR(VER_PATCH)
#endif //APPKILLER_VERSION_H
