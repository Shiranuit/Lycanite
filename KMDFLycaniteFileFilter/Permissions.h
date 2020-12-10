#pragma once

#include "IKashmap.h"

#if defined(__cplusplus)
extern "C" {
#endif

	static UINT32 my_strlen(CONST PCHAR str);

	static UINT64 getFilePermission(CONST PCHAR path, CONST struct hashmap_s* map);
	static UINT64 getFilePermissionWithFree(CONST PCHAR path, CONST struct hashmap_s* map);

	static PCHAR hasParentFolder(CONST PCHAR path, CONST struct hashmap_s* map);

#if defined(__cplusplus)
}
#endif

UINT32 my_strlen(CONST PCHAR str)
{
	UINT32 len = 0;

	for (SIZE_T i = 0; str[i] != '\0'; i++) {
		len += 1;
	}
	return len;
}

UINT64 getFilePermission(CONST PCHAR path, CONST struct hashmap_s* map)
{
	PVOID path_permissions = hashmap_get(map, path, my_strlen(path));

	if (path_permissions != HASHMAP_NULL) {
		return *(UINT64*)path_permissions;
	}
	else {
		PCHAR parent = NULL;
		/* ajouter un bool ou une connerie du genre pour free parent quand la récursive se fait */
		if ((parent = hasParentFolder(path, map)) != NULL) {
			return getFilePermissionWithFree(parent, map);
		}
		else {
			return 0; // return no permissions
		}
	}
}

UINT64 getFilePermissionWithFree(CONST PCHAR path, CONST struct hashmap_s* map)
{
	PVOID path_permissions = hashmap_get(map, path, my_strlen(path));

	if (path_permissions != HASHMAP_NULL) {
		free((PVOID)path);
		return *(UINT64*)path_permissions;
	}
	else {
		PCHAR parent = NULL;
		/* ajouter un bool ou une connerie du genre pour free parent quand la récursive se fait */
		if ((parent = hasParentFolder(path, map)) != NULL) {
			return getFilePermissionWithFree(parent, map);
		}
		else {
			return 0; // return no permissions
		}
	}
}

PCHAR hasParentFolder(CONST PCHAR path, CONST struct hashmap_s* map)
{
	SIZE_T memsize = my_strlen(path);
	PCHAR parent = (PCHAR)calloc(memsize, 1);

	Kmemcpy((PVOID)parent, (PVOID)path, memsize);
	for (SIZE_T i = memsize - 2; i > 0; i--) {
		if (((PCHAR)parent)[i] == '\\') {
			parent[i] = '\0';
			if (hashmap_get(map, parent, my_strlen(parent)) != HASHMAP_NULL) {
				return parent;
			}
		}
	}
	free(parent);
	return NULL;
}