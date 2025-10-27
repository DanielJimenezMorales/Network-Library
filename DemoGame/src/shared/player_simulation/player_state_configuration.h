#pragma once
#include "numeric_types.h"

namespace PlayerSimulation
{
	struct PlayerStateConfiguration
	{
			PlayerStateConfiguration( uint32 movement_speed, float32 aiming_movement_speed_multiplier,
			                          uint32 fire_rate_per_second )
			    : _movementSpeed( movement_speed )
			    , _aimingMovementSpeedMultiplier( aiming_movement_speed_multiplier )
			    , _fireRatePerSecond( fire_rate_per_second )
			    , _fireRate( 1.f / fire_rate_per_second )
			{
			}

			inline uint32 GetMovementSpeed() const { return _movementSpeed; }
			inline float32 GetAimingMovementSpeedMultiplier() const { return _aimingMovementSpeedMultiplier; }
			inline uint32 GetFireRatePerSecond() const { return _fireRatePerSecond; }
			inline float32 GetFireRate() const { return _fireRate; }

		private:
			uint32 _movementSpeed;
			float32 _aimingMovementSpeedMultiplier;
			// This value can't be higher than the game Tick Rate
			uint32 _fireRatePerSecond;
			float32 _fireRate;
	};
} // namespace PlayerSimulation
