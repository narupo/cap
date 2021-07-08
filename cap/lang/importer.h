#pragma once

#include <pad/lib/file.h>

#include <cap/core/config.h>
#include <cap/core/util.h>

void
CapImporter_SetCapConfig(const CapConfig *config);

char *
CapImporter_FixPath(PadImporter *self, char *dst, int32_t dstsz, const char *cap_path);
