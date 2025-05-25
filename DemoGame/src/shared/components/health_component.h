#pragma once
#include "numeric_types.h"

struct HealthComponent
{
	public:
		HealthComponent();

		uint32 maxHealth;
		uint32 currentHealth;
};
