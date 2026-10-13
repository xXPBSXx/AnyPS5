# libSceSysmodule technical debt

### Silent stubs

- [sceSysmoduleLoadModule, sceSysmoduleIsLoaded, sceSysmoduleUnloadModule](../../../core/libs/prx/libSceSysmodule/Export.cpp) (libSceSysmodule) log `unknown id` and return 0 for a module id that is not in [the module table](../../../core/libs/prx/libSceSysmodule/ModuleTable.hpp), so `sceSysmoduleIsLoaded` reports such a module as loaded and the call does not fail; the `Internal` variants throw instead. Ids seen this way in saved title runs: 0x127 (295) and 0x126 (294); their module names are not known. None of this was measured on a console.
