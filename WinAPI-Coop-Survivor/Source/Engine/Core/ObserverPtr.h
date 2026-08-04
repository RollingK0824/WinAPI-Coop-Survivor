#pragma once
template <typename T>
class ObserverPtr
{
public:
    ObserverPtr(T* ptr = nullptr) : m_ptr(ptr) {}

    ObserverPtr& operator=(const ObserverPtr& other)
    {
        m_ptr = nullptr;
        return *this;
    }

    ObserverPtr& operator=(T* ptr)
    {
        m_ptr = ptr;
        return *this;
    }

    T* operator->() const { return m_ptr; }
    T& operator*() const { return *m_ptr; }
    T* Get() const { return m_ptr; }
    bool IsValid() const { return m_ptr != nullptr; }

private:
    T* m_ptr = nullptr;
};