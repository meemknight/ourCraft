#pragma once
//////////////////////////////////////////
//staticVector.h
//Luta Vlad(c) 2022
//https://github.com/meemknight/PikaEngine
//////////////////////////////////////////

#include <initializer_list>
#include <assert.h>

template<class T, size_t N>
struct StaticVector
{
	typedef T *iterator;
	typedef const T *constIterator;

	iterator begin() { return &((T *)beg_)[0]; }
	constIterator begin() const { return &((T *)beg_)[0]; }
	iterator end() { return &((T *)beg_)[size_]; }
	constIterator end() const { return &((T *)beg_)[size_]; }

	static constexpr unsigned int MAX_SIZE = N;
	static constexpr unsigned int capacity = N;

	StaticVector() {};

	StaticVector(std::initializer_list<T> &&l)
	{
		for (auto &i : l)
		{
			push_back(i);
		}
	};

	StaticVector(StaticVector &&other)
	{
		for (size_t i = 0; i < other.size_; i++)
		{
			beg_[i] = std::move(other.beg_[i]);
		}
		
		this->size_ = other.size_;
		other.size_ = 0;
	}

	StaticVector(const StaticVector &other)
	{
		for (size_t i = 0; i < other.size_; i++)
		{
			beg_[i] = other.beg_[i];
		}

		this->size_ = other.size_;
	}

	size_t size()const { return size_; }

	bool empty() const
	{
		return (size_ == 0);
	}

	T *data()
	{
		return beg_;
	}

	StaticVector &operator= (const StaticVector &other)
	{
		if (this == &other)
		{
			return *this;
		}

		for (size_t i = 0; i < other.size_; i++)
		{
			beg_[i] = other.beg_[i];
		}
		this->size_ = other.size_;

		return *this;
	}

	StaticVector &operator= (StaticVector &&other)
	{
		if (this == &other)
		{
			return *this;
		}

		for (size_t i = 0; i < other.size_; i++)
		{
			beg_[i] = std::move(other.beg_[i]);
		}

		this->size_ = other.size_;
		other.size_ = 0;

		return *this;
	}

	bool operator==(const StaticVector &other)
	{
		if (this == &other) { return true; }

		if (this->size_ != other.size_) { return false; }

		for (int i = 0; i < size_; i++)
		{
			if ( (*this)[i] != other[i])
			{
				return false;
			}
		}
		
		return true;
	}

	bool operator!=(const StaticVector &other)
	{
		return !(*this == other);
	}

	T &operator[] (size_t index)
	{
		//permaAssertComment(index < size_, "buffer overflow on acces");
		return (beg_)[index];
	}

	T operator[] (size_t index) const
	{
		//permaAssertComment(index < size_, "buffer overflow on acces");
		return (beg_)[index];
	}

	T &back()
	{
		return (*this)[size_ - 1];
	}

	const T &back() const
	{
		return (*this)[size_ - 1];
	}

	void clear() { size_ = 0; }

	void push_back(const T &el)
	{
		assert(size_ < MAX_SIZE, "exceded max size in push back");
		beg_[size_] = el;
		size_++;
	}

	void push_back(T &&el)
	{
		assert(size_ < MAX_SIZE, "exceded max size in push back");
		beg_[size_] = std::forward<T>(el);
		size_++;
	}

	void pop_back()
	{
		assert(size_ > 0, "buffer underflow on pop back");
		size_--;
	}


	size_t size_ = 0;
	T beg_[N];

};


