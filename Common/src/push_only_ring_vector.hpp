#pragma once
#include "numeric_types.h"

/// <summary>
/// <para>Ring buffer where elements can only be pushed, not poped. When it is full, it will go to the next element and
/// overwrite it.</para> <para>Requirements for T: 1) Empty, Copy and Move constructors available, 2) Copy and move
/// assigment operators available</para>
/// </summary>
template < typename T, uint32 MaxSize >
class PushOnlyRingVector
{
	public:
		class ConstIterator
		{
			public:
				ConstIterator( const T* data, uint32 index, bool first_iteration )
				    : _dataBegin( data )
				    , _index( index )
				    , _firstIteration( first_iteration )
				{
				}

				const T& operator*() const { return _dataBegin[ _index ]; }
				const T* operator->() const { return &_dataBegin[ _index ]; }

				bool operator==( const PushOnlyRingVector< T, MaxSize >::ConstIterator& other ) const
				{
					return _dataBegin == other._dataBegin && _index == other._index &&
					       _firstIteration == other._firstIteration;
				}

				bool operator!=( const PushOnlyRingVector< T, MaxSize >::ConstIterator& other ) const
				{
					return !( *this == other );
				}

				PushOnlyRingVector< T, MaxSize >::ConstIterator& operator++()
				{
					if ( _firstIteration )
					{
						_firstIteration = false;
					}

					_index = ( _index + 1 ) % MaxSize;
					return *this;
				}

				PushOnlyRingVector< T, MaxSize >::ConstIterator operator++( int )
				{
					if ( _firstIteration )
					{
						_firstIteration = false;
					}

					PushOnlyRingVector< T, MaxSize >::ConstIterator temp = *this;
					_index = ( _index + 1 ) % MaxSize;
					return temp;
				}

				PushOnlyRingVector< T, MaxSize >::ConstIterator& operator--() = delete;

				PushOnlyRingVector< T, MaxSize >::ConstIterator operator--( int ) = delete;

			private:
				const T* _dataBegin;
				uint32 _index;
				bool _firstIteration;
		};

		PushOnlyRingVector();
		PushOnlyRingVector( const PushOnlyRingVector< T, MaxSize >& other );
		PushOnlyRingVector( PushOnlyRingVector< T, MaxSize >&& other ) noexcept;
		~PushOnlyRingVector() = default;

		/// <summary>
		/// Returns a constant iterator to the first element of the vector. If the vector is empty, a constant
		/// past-the-end iterator will be returned.
		/// </summary>
		PushOnlyRingVector< T, MaxSize >::ConstIterator CBegin() const
		{
			return PushOnlyRingVector< T, MaxSize >::ConstIterator( m_data, _index, false );
		}

		/// <summary>
		/// Returns a constant past-the-end iterator.
		/// returned.
		/// </summary>
		PushOnlyRingVector< T, MaxSize >::ConstIterator CEnd() const
		{
			return PushOnlyRingVector< T, MaxSize >::ConstIterator( m_data, ( _index + _size ) % MaxSize, true );
		}

		/// <summary>
		/// <para>Returns true if the content of both vectors is the same.</para>
		/// </summary>
		/// <param name="other">The vector to compare with</param>
		/// <returns>True if they are different, False otherwise</returns>
		bool operator==( const PushOnlyRingVector< T, MaxSize >& other ) const;

		/// <summary>
		/// <para>Returns true if the content of both vectors is different.</para>
		/// </summary>
		/// <param name="other">The vector to compare with</param>
		/// <returns>True if they are the same, False otherwise</returns>
		bool operator!=( const PushOnlyRingVector< T, MaxSize >& other ) const;

		bool Empty() const;
		uint32 Size() const;

		/// <summary>
		/// <para>Returns a pointer to the oldest element within the circular vector.</para>
		/// <para>Important Note: The returned pointer is temporary. Do not store it. Regarding T, copy or move it as
		/// soon as possible if that's the intention.</para>
		/// </summary>
		const T* Front() const;

		/// <summary>
		/// <para>Adds a new element to the vector</para>
		/// <para>Note: If the vector is full the element will not be added and Push will return false</para>
		/// </summary>
		bool Push( const T& item );

		/// <summary>
		/// <para>Adds a new element to the vector</para>
		/// <para>Note: If the vector is full the element will not be added and Push will return false</para>
		/// </summary>
		bool Push( T&& item );

		/// <summary>
		/// Inserts a new element in the head of the circular vector. This method constructs the element in place.
		/// Returns if the element was succesfully added. If Empty, this method will return false.
		/// </summary>
		template < class... Args >
		bool Emplace( Args&&... args );

		/// <summary>
		/// <para>Clears the vector</para>
		/// <para>Note: This method does not free memory.</para>
		/// </summary>
		void Clear();

	private:
		void IncreaseIndex( uint32& index ) const;

		T m_data[ MaxSize ];
		uint32 _index;
		uint32 _size;

		static constexpr uint32 MINIMUM_SIZE_ALLOWED = 2;
};

template < typename T, uint32 MaxSize >
inline PushOnlyRingVector< T, MaxSize >::PushOnlyRingVector()
    : m_data()
    , _size( 0 )
    , _index( 0 )
{
	static_assert( MaxSize > 0 );
	// STATIC_ASSERT( MaxSize >= MINIMUM_SIZE_ALLOWED, "Circular buffer's max size must be greater than 1" );
}

template < typename T, uint32 MaxSize >
inline PushOnlyRingVector< T, MaxSize >::PushOnlyRingVector( const PushOnlyRingVector< T, MaxSize >& other )
    : PushOnlyRingVector()
{
	static_assert( MaxSize > 0 );
	// STATIC_ASSERT( MaxSize >= MINIMUM_SIZE_ALLOWED, "Circular buffer's max size must be greater than 1" );

	auto cit = other.CBegin();
	for ( ; cit != other.CEnd(); ++cit )
	{
		Push( *cit );
	}
}

template < typename T, uint32 MaxSize >
inline PushOnlyRingVector< T, MaxSize >::PushOnlyRingVector( PushOnlyRingVector< T, MaxSize >&& other ) noexcept
    : PushOnlyRingVector()
{
	static_assert( MaxSize > 0 );
	// STATIC_ASSERT( MaxSize >= MINIMUM_SIZE_ALLOWED, "Circular buffer's max size must be greater than 1" );

	_index = other._index;
	const uint32 targetSize = ( other._size >= MaxSize ) ? MaxSize : other._size;
	while ( _size < targetSize )
	{
		Push( std::move( other.m_data[ _index ] ) );
	}

	uint32 read_index = other.m_readIndex;
	uint32 size = 0;
	while ( read_index != other.m_writeIndex || size < other._size )
	{
		Push( std::move( other.m_data[ read_index ] ) );
		IncreaseIndex( read_index );
		++size;
	}

	other._size = other.m_readIndex = other.m_writeIndex = 0;
}

template < typename T, uint32 MaxSize >
inline bool PushOnlyRingVector< T, MaxSize >::operator==( const PushOnlyRingVector< T, MaxSize >& other ) const
{
	bool equal = true;
	if ( _size != other._size )
	{
		equal = false;
	}
	else
	{
		uint32 index = _index;
		for ( auto cit = other.CBegin(); cit != other.CEnd(); ++cit )
		{
			if ( m_data[ index ] != *cit )
			{
				equal = false;
				break;
			}

			IncreaseIndex( index );
		}
	}

	return equal;
}

template < typename T, uint32 MaxSize >
inline bool PushOnlyRingVector< T, MaxSize >::Empty() const
{
	return _size == 0;
}

template < typename T, uint32 MaxSize >
inline uint32 PushOnlyRingVector< T, MaxSize >::Size() const
{
	return _size;
}

template < typename T, uint32 MaxSize >
inline const T* PushOnlyRingVector< T, MaxSize >::Front() const
{
	if ( Empty() )
	{
		return nullptr;
	}

	return &m_data[ m_readIndex ];
}

template < typename T, uint32 MaxSize >
inline bool PushOnlyRingVector< T, MaxSize >::Push( const T& item )
{
	++_size;
	m_data[ _index ] = item;
	IncreaseIndex( _index );

	return true;
}

template < typename T, uint32 MaxSize >
inline bool PushOnlyRingVector< T, MaxSize >::Push( T&& item )
{
	++_size;
	m_data[ _index ] = std::move( item );
	IncreaseIndex( _index );

	return true;
}

template < typename T, uint32 MaxSize >
template < class... Args >
inline bool PushOnlyRingVector< T, MaxSize >::Emplace( Args&&... args )
{
	return Push( std::move( T( std::forward< Args >( args )... ) ) );
}

template < typename T, uint32 MaxSize >
inline void PushOnlyRingVector< T, MaxSize >::Clear()
{
	m_readIndex = m_writeIndex = _size = 0;
}

template < typename T, uint32 MaxSize >
inline void PushOnlyRingVector< T, MaxSize >::IncreaseIndex( uint32& index ) const
{
	if ( index == ( MaxSize - 1 ) )
	{
		index = 0;
	}
	else
	{
		++index;
	}
}
