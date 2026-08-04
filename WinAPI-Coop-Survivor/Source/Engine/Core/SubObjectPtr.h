#pragma once

template<typename T>
class SubObjectPtr
{
public:
	SubObjectPtr(T* ptr = nullptr) : m_ptr(ptr) {}
	~SubObjectPtr()
	{
		if (m_ptr)
		{
			delete m_ptr;
			m_ptr = nullptr;
		}
	}

	SubObjectPtr(const SubObjectPtr& other)
	{
		m_ptr = other.m_ptr ? static_cast<T*>(other.m_ptr->Clone()) : nullptr;
	}

	SubObjectPtr(SubObjectPtr&& other) noexcept : m_ptr(other.m_ptr)
	{
		other.m_ptr = nullptr;
	}

	SubObjectPtr& operator=(const SubObjectPtr& other)
	{
		if (this != &other)
		{
			if (m_ptr)delete m_ptr;
			m_ptr = other.m_ptr ? static_cast<T*>(other.m_ptr->Clone()) : nullptr;
		}
		return this;
	}

	SubObjectPtr& operator=(SubObjectPtr&& other) noexcept
	{
		if (this != &other)
		{
			if (m_ptr) delete m_ptr;
			m_ptr = other.m_ptr;
			other.m_ptr = nullptr;
		}
		return *this;
	}

	T* operator->() const { return m_ptr; }
	T& operator*() const { return *m_ptr; }
	T* Get() const { return m_ptr; }
	bool IsValid() const { return m_ptr != nullptr; }
private:
	T* m_ptr = nullptr;
};