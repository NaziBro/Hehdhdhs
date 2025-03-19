#pragma once

#include "json/json.h"


class BlockRuntimeIDMap {
public:
	Json::Value ridMap117;
	Json::Value ridMap118;
	Json::Value schematicArray;
	Json::Value schemArray;

	int	rt_aux117 = 20017;
	int	rt_aux118 = 20018;
	int rt_schemfile = 20019;
	BlockRuntimeIDMap();
};