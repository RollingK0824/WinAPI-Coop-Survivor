#pragma once
#include <type_traits>

class GameObject;

template <typename T>
class ObserverPtr
{
public:
    ObserverPtr(T* ptr = nullptr) : m_ptr(ptr)
    {
        Register();
    }

    ~ObserverPtr()
    {
        Unregister();
    }

    ObserverPtr(const ObserverPtr& other) : m_ptr(other.m_ptr)
    {
        Register();
    }

    ObserverPtr& operator=(const ObserverPtr& other)
    {
        if (this != &other)
        {
            Unregister();
            m_ptr = other.m_ptr;
            Register();
        }
        return *this;
    }

    ObserverPtr& operator=(T* ptr)
    {
        if (m_ptr != ptr)
        {
            Unregister();
            m_ptr = ptr;
            Register();
        }
        return *this;
    }

    T* operator->() const { return m_ptr; }
    T& operator*() const { return *m_ptr; }
    T* Get() const { return m_ptr; }
    bool IsValid() const 
    { 
        if (!m_ptr) return false;
        return true; 
    }

    explicit operator bool() const { return IsValid(); }

private:
    void Register();
    void Unregister();

private:
    T* m_ptr = nullptr;
};

#include "Engine/Framework/GameObject.h"

template <typename T>
inline void ObserverPtr<T>::Register()
{
    if (m_ptr)
    {
        if constexpr (std::is_same_v<std::remove_cv_t<T>, GameObject>)
        {
            reinterpret_cast<GameObject*>(m_ptr)->RegisterObserverPtr(reinterpret_cast<void**>(&m_ptr));
        }
    }
}

template <typename T>
inline void ObserverPtr<T>::Unregister()
{
    if (m_ptr)
    {
        if constexpr (std::is_same_v<std::remove_cv_t<T>, GameObject>)
        {
            reinterpret_cast<GameObject*>(m_ptr)->UnregisterObserverPtr(reinterpret_cast<void**>(&m_ptr));
        }
    }
}