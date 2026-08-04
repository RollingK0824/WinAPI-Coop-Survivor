#pragma once
#include "Engine/Core/pch.h"
#include "Engine/Core/Singleton.h"
#include "Engine/Framework/Base/ISystem.h"

template<typename T>
class ObjectPool
{
public:
	using CreateFunc = std::function<T*()>;
	using ActionFunc = std::function<void(T*)>;

	ObjectPool(CreateFunc createFunc,
		ActionFunc actionOnGet = nullptr,
		ActionFunc actionOnRelease = nullptr,
		ActionFunc actionOnDestroy = nullptr,
		size_t defaultCapacity = 10,
		size_t maxSize = 1000)
		: m_createFunc(createFunc)
		, m_actionOnGet(actionOnGet)
		, m_actionOnRelease(actionOnRelease)
		, m_actionOnDestroy(actionOnDestroy)
		, m_maxSize(maxSize)
	{
		m_pool.reserve(defaultCapacity);
		for (size_t i = 0; i < defaultCapacity; ++i)
		{
			T* obj = m_createFunc();
			if (m_actionOnRelease) m_actionOnRelease(obj);
			m_pool.push_back(obj);
		}
	}

	~ObjectPool()
	{
		Clear();
	}

	T* Get()
	{
		T* obj = nullptr;
		if (m_pool.empty())
		{
			obj = m_createFunc();
		}
		else
		{
			obj = m_pool.back();
			m_pool.pop_back();
		}
		if (m_actionOnGet) m_actionOnGet(obj);
		return obj;
	}

	void Release(T* obj)
	{
		if (obj == nullptr) return;
		if (m_actionOnRelease) m_actionOnRelease(obj);
		if (m_pool.size() < m_maxSize)
		{
			m_pool.push_back(obj);
		}
		else
		{
			if (m_actionOnDestroy) m_actionOnDestroy(obj);
			delete obj;
		}
	}

	void Clear()
	{
		for (T* obj : m_pool)
		{
			if (m_actionOnDestroy) m_actionOnDestroy(obj);
			delete obj;
		}
		m_pool.clear();
	}
	size_t GetCountInactive() const { return m_pool.size(); }

private:
	CreateFunc m_createFunc;
	ActionFunc m_actionOnGet;
	ActionFunc m_actionOnRelease;
	ActionFunc m_actionOnDestroy;
	std::vector<T*> m_pool;
	size_t m_maxSize;
};

class IPoolContainer
{
public:
	virtual ~IPoolContainer() = default;
	virtual void Clear() = 0;
};

template<typename T>
class PoolContainer : public IPoolContainer
{
public:
	PoolContainer(std::unique_ptr<ObjectPool<T>> pool)
		: m_pool(std::move(pool)) {
	}
	ObjectPool<T>* GetPool() { return m_pool.get(); }
	virtual void Clear() override { m_pool->Clear(); }
private:
	std::unique_ptr<ObjectPool<T>> m_pool;
};

class PoolManager : public Singleton<PoolManager>, public ISystem
{
	friend class Singleton<PoolManager>;
public:
	virtual bool Initialize() override { return true; }
	virtual void Release() override { ClearAll(); }

	template<typename T>
	void CreatePool(const std::string& key,
		typename ObjectPool<T>::CreateFunc createFunc,
		typename ObjectPool<T>::ActionFunc actionOnGet = nullptr,
		typename ObjectPool<T>::ActionFunc actionOnRelease = nullptr,
		typename ObjectPool<T>::ActionFunc actionOnDestroy = nullptr,
		size_t defaultCapacity = 10,
		size_t maxSize = 1000)
	{
		auto pool = std::make_unique<ObjectPool<T>>(createFunc, actionOnGet, actionOnRelease, actionOnDestroy, defaultCapacity, maxSize);
		m_pools[key] = std::make_unique<PoolContainer<T>>(std::move(pool));
	}

	template<typename T>
	T* Spawn(const std::string& key)
	{
		auto it = m_pools.find(key);
		if (it == m_pools.end()) return nullptr;
		auto* container = static_cast<PoolContainer<T>*>(it->second.get());
		return container->GetPool()->Get();
	}

	template<typename T>
	void Despawn(const std::string& key, T* obj)
	{
		auto it = m_pools.find(key);
		if (it == m_pools.end()) return;
		auto* container = static_cast<PoolContainer<T>*>(it->second.get());
		container->GetPool()->Release(obj);
	}

	void ClearAll()
	{
		for (auto& pair : m_pools)
		{
			pair.second->Clear();
		}
		m_pools.clear();
	}

private:
	PoolManager() = default;
	virtual ~PoolManager() override = default;
	std::unordered_map<std::string, std::unique_ptr<IPoolContainer>> m_pools;
};