#pragma once

template <typename T>
class SafePointer
{
public:
	SafePointer(T* rawPointer) : _rawPointer(rawPointer)
	{
	}

	SafePointer(const SafePointer<T>& other) : _rawPointer(other._rawPointer)
	{
	}

	T& operator*()
	{
		return *_rawPointer;
	}

	const T& operator*() const
	{
		return *_rawPointer;
	}

	T* operator->()
	{
		return _rawPointer;
	}

	const T* operator->() const
	{
		return _rawPointer;
	}

	bool operator==(const T* other) const
	{
		return _rawPointer == other;
	}

	bool operator!=(const T* other) const
	{
		return !(*this == other);
	}

	bool operator==(const SafePointer<T>& other) const
	{
		return _rawPointer == other._rawPointer;
	}

	bool operator!=(const SafePointer<T>& other) const
	{
		return !(*this == other);
	}


	T* GetRawPointer();

private:
	T* _rawPointer;
};

template<typename T>
inline T* SafePointer<T>::GetRawPointer()
{
	return _rawPointer;
}
