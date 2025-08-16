#pragma once
#include "server/hit_reg/shot_entry.h"

#include <queue>

struct HitRegistrationGlobalComponent
{
		std::queue< ShotEntry > pendingShotEntries;
		float32 maxAllowedRollbackTimeSeconds = 0.25f;
};
