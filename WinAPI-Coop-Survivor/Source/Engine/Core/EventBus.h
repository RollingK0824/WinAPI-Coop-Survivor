#pragma once
#include "Engine/Core/pch.h"
#include "Engine/Core/Singleton.h"
#include "Engine/Framework/Base/ISystem.h"

class IEventSubscriptionList
{
public:
	virtual ~IEventSubscriptionList() = default;
};

template<typename T>
class EventSubscriptionList : public IEventSubscriptionList
{
public:
	using Callback = std::function<void(const T&)>;
	using ListenerID = size_t;

	struct Subscription
	{
		ListenerID  id;
		Callback callback;
	};

	ListenerID Subscribe(Callback callback)
	{
		ListenerID  id = m_nextId++;
		m_subscriptions.push_back({ id,std::move(callback) });
		return id;
	}

	void Unsubscribe(ListenerID id)
	{
		for (auto it = m_subscriptions.begin(); it != m_subscriptions.end(); ++it)
		{
			if (it->id == id)
			{
				m_subscriptions.erase(it);
				break;
			}
		}
	}

	void Publish(const T& event)
	{
		auto copyList = m_subscriptions;
		for (const auto& sub : copyList)
		{
			if (sub.callback)
			{
				sub.callback(event);
			}
		}
	}

	void Clear()
	{
		m_subscriptions.clear();
	}

private:
	std::vector<Subscription> m_subscriptions;
	ListenerID m_nextId = 1;
};

class EventBus : public Singleton<EventBus>, public ISystem
{
	friend class Singleton<EventBus>;
public:
	using ListenerID = size_t;

	virtual bool Initialize() override { return true; }
	virtual void Release() override { ClearAll(); }

	template<typename T>
	ListenerID Subscribe(std::function<void(const T&)> callback)
	{
		auto typeKey = std::type_index(typeid(T));
		if (m_subscribers.find(typeKey) == m_subscribers.end())
		{
			m_subscribers[typeKey] = std::make_unique<EventSubscriptionList<T>>();
		}
		auto* list = static_cast<EventSubscriptionList<T>*>(m_subscribers[typeKey].get());
		return list->Subscribe(callback);
	}

	template<typename T>
	void Unsubscribe(ListenerID id)
	{
		auto typeKey = std::type_index(typeid(T));
		auto it = m_subscribers.find(typeKey);
		if (it != m_subscribers.end())
		{
			auto* list = static_cast<EventSubscriptionList<T>*>(it->second.get());
			list->Unsubscribe(id);
		}
	}

	template<typename T>
	void Publish(const T& event)
	{
		auto typeKey = std::type_index(typeid(T));
		auto it = m_subscribers.find(typeKey);
		if (it != m_subscribers.end())
		{
			auto* list = static_cast<EventSubscriptionList<T>*>(it->second.get());
			list->Publish(event);
		}
	}

	void ClearAll()
	{
		m_subscribers.clear();
	}
private:
	EventBus() = default;
	virtual ~EventBus() = default;

	std::unordered_map<std::type_index, std::unique_ptr<IEventSubscriptionList>>m_subscribers;
};