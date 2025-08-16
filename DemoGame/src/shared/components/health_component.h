#pragma once
#include "numeric_types.h"

struct HealthComponent
{
	public:
		HealthComponent();

		void ApplyDamage( uint32 damage )
		{
			if ( currentHealth > damage )
			{
				currentHealth -= damage;
			}
			else
			{
				currentHealth = 0;
			}
		}

		uint32 maxHealth;
		uint32 currentHealth;
};
