#pragma once

#include "IKashmap.h"

#if defined(__cplusplus)
extern "C" {
#endif

	static UINT64 getFilePermission(CONST PWCHAR path, CONST struct hashmap_s* map);
	static UINT64 getFilePermissionWithFree(CONST PWCHAR path, CONST struct hashmap_s* map);

	static PWCHAR hasParentFolder(CONST PWCHAR path, CONST struct hashmap_s* map);

#if defined(__cplusplus)
}
#endif

UINT64 getFilePermission(CONST PWCHAR path, CONST struct hashmap_s* map)
{
	PVOID path_permissions = hashmap_get(map, path, my_strlen(path));

	if (path_permissions != HASHMAP_NULL) {
		return *(UINT64*)path_permissions;
	}
	else {
		PWCHAR parent = NULL;
		if ((parent = hasParentFolder(path, map)) != NULL) {
			return getFilePermissionWithFree(parent, map);
		}
		else {
			return 0; // return no permissions
		}
	}
}

UINT64 getFilePermissionWithFree(CONST PWCHAR path, CONST struct hashmap_s* map)
{
	PVOID path_permissions = hashmap_get(map, path, my_strlen(path));

	if (path_permissions != HASHMAP_NULL) {
		free((PVOID)path);
		return *(UINT64*)path_permissions;
	}
	else {
		PWCHAR parent = NULL;
		if ((parent = hasParentFolder(path, map)) != NULL) {
			return getFilePermissionWithFree(parent, map);
		}
		else {
			free((PVOID)path);
			return 0; // return no permissions
		}
	}
}

PWCHAR hasParentFolder(CONST PWCHAR path, CONST struct hashmap_s* map)
{
	SIZE_T memsize = my_strlen(path) + 1;
	PWCHAR parent = (PWCHAR)calloc(memsize, sizeof(WCHAR));

	Kmemcpy((PVOID)parent, (PVOID)path, memsize);
	for (SIZE_T i = memsize - 2; i > 0; i--) {
		if (((PWCHAR)parent)[i] == '\\') {
			parent[i] = '\0';
			if (hashmap_get(map, parent, my_strlen(parent)) != HASHMAP_NULL) {
				return parent;
			}
		}
	}
	free(parent);
	return NULL;
}