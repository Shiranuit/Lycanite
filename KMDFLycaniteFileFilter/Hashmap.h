#pragma once

#include "CPP.h"

class Hashmap {
public:
	Hashmap() {
		KdPrint(("%s\n", "NEW"));
	}
	~Hashmap() {
		KdPrint(("%s\n", "DELETE"));
	}
};