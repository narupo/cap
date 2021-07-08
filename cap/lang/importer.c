#include <lang/importer.h>

static CapConfig *_cap_config;

void
CapImporter_SetCapConfig(const CapConfig *config) {
    _cap_config = config;
}

char *
CapImporter_FixPath(PadImporter *imptr, char *dst, int32_t dstsz, const char *cap_path) {
    if (!dst || dstsz <= 0 || !cap_path) {
        PadImporter_SetErr(imptr, "invalid arguments");
        return NULL;
    }

    if (!Cap_SolveCmdlineArgPath(_cap_config, dst, dstsz, cap_path)) {
        PadImporter_SetErr(imptr, "failed to solve cap path of \"%s\"", cap_path);
        return NULL;
    }

    // check cap_path
    if (!PadFile_IsExists(dst)) {
        // create cap_path of standard libraries
        // will read source from standard library module
        if (!PadFile_SolveFmt(dst, dstsz, "%s/%s", _cap_config->std_lib_dir_path, cap_path)) {
            PadImporter_SetErr(imptr, "failed to solve path for standard library");
            return NULL;
        }
        if (!file_exists(dst)) {
            PadImporter_SetErr(imptr, "\"%s\" is not found", cap_path);
            return NULL;
        }
    }

    return dst;
}
