#pragma once
#include "numeric_types.h"

#include "metrics/i_metric.h"

#include <vector>
#include <unordered_map>
#include <memory>
#include <string>

namespace NetLib
{
	namespace Metrics
	{
		class MetricsHandler
		{
			public:
				MetricsHandler();

				void Configure( float32 update_rate );

				void Update( float32 elapsed_time );

				/// <summary>
				/// Add a network statistic entry. This class is responsible for handling the entries memory.
				/// </summary>
				bool AddEntry( std::unique_ptr< IMetric > entry );

				/// <summary>
				/// Gets the value of type value_type (MAX, CURRENT...) from the entry with name entry_name. If the
				/// entry name or the value type is invalid the function returns 0.
				/// </summary>
				uint32 GetValue( const std::string& entry_name, const std::string& value_type ) const;

				/// <summary>
				/// Adds a value to the entry with name entry_name. If the entry doesn't exists it does nothing and
				/// returns false.
				/// </summary>
				bool AddValue( const std::string& entry_name, uint32 value );

			private:
				void Reset();
				std::unordered_map< std::string, std::unique_ptr< IMetric > > _entries;
		};
	}
} // namespace NetLib