#pragma once

#include "Utils.h"

#define UUID UINT64

typedef struct UUIDRecyler_s {
	UINT64 size;
	UINT64 capacity;
	UUID* uuids;
	UUID lastID;
} UUIDRecycler;

UUIDRecycler* UUIDRecycler_create(UINT64 initialCapacity) {
	UINT64 capacity = initialCapacity > 0 ? initialCapacity : 16;
	UUIDRecycler* recycler = (UUIDRecycler*)calloc(1, sizeof(UUIDRecycler));
	if (recycler != NULL) {
		recycler->size = 0;
		recycler->capacity = capacity;
		recycler->lastID = 1;
		recycler->uuids = (UUID*)calloc(capacity, sizeof(UUID));
		if (recycler->uuids != NULL) {
			return recycler;
		}
		free(recycler);
	}
	return NULL;
}

void UUIDRecycler_recycleUUID(UUIDRecycler* recycler, UUID uuid) {
	if (recycler != NULL) {
		if (recycler->size + 1 > recycler->capacity) {
			recycler->capacity += (recycler->capacity >> 1) | 1;
			UUID* newUUIDs = (UUID*)calloc(recycler->capacity, sizeof(UUID));
			RtlCopyMemory(newUUIDs, recycler->uuids, recycler->size * sizeof(UUID));
			free(recycler->uuids);
			recycler->uuids = newUUIDs;
		}
		recycler->uuids[recycler->size++] = uuid;
	}
}


UUID UUIDRecycler_getUUID(UUIDRecycler* recycler) {
	if (recycler != NULL) {
		if (recycler->size == 0) {
			return recycler->lastID++;
		}
		return recycler->uuids[--recycler->size];
	}
	return 0;
}


void UUIDRecycler_destroy(UUIDRecycler* recycler) {
	if (recycler != NULL) {
		free(recycler->uuids);
		free((PVOID)recycler);
	}
}
