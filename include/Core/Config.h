#pragma once

#if _WIN32
#define SH_API_EXPORT __declspec(dllexport)
#define SH_API_IMPORT __declspec(dllimport)
#elif __linux__
#define SH_API_EXPORT __attribute__((__visibility__("default")))
#define SH_API_IMPORT __attribute__((__visibility__("default")))
#endif