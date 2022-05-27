#include <Windows.h>
#include "Dll.h"

HINSTANCE g_hinst = NULL; // global dll hinstance

STDAPI_(BOOL) DllMain(__in HINSTANCE hinstDll, __in DWORD dwReason, __in void *)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hinstDll);
		break;
	case DLL_PROCESS_DETACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}

	g_hinst = hinstDll;
	return TRUE;
}
