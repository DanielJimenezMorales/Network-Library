#pragma once
#include "numeric_types.h"

#include "vec2f.h"

#include <unordered_map>

struct HistoryEntry
{
		HistoryEntry()
		    : position()
		    , rotationAngle( 0.f )
		{
		}

		Vec2f position;
		float32 rotationAngle;
};

static constexpr uint32 MAX_HISTORY_BUFFER_SIZE = 30;

/// <summary>
/// This component stores the history of position + rotation data from previous simulation ticks in order to be able to
/// rewind for HitRegistration
/// </summary>
struct ServerTransformHistoryComponent
{
		ServerTransformHistoryComponent()
		    : serverTimeBuffer()
		    , historyBuffer()
		    , currentIndex( 0 )
		{
			serverTimeBuffer.reserve( MAX_HISTORY_BUFFER_SIZE );
			historyBuffer.reserve( MAX_HISTORY_BUFFER_SIZE );
			for ( uint32 i = 0; i < MAX_HISTORY_BUFFER_SIZE; ++i )
			{
				serverTimeBuffer.push_back( 0.f );
				historyBuffer.emplace_back();
			}
		}

		uint32 GetCurrentIndexMinusOne() const
		{
			return ( currentIndex == 0 ) ? MAX_HISTORY_BUFFER_SIZE - 1 : currentIndex - 1;
		}

		// TODO Create custon Ring Vector class
		std::vector< float32 > serverTimeBuffer;
		std::vector< HistoryEntry > historyBuffer;
		uint32 currentIndex;
};
