#include "SObject.h"

#include "GarbageCollection.h"
#include "Observer.hpp"

#include <cstring>

namespace sh::core
{
	SObject::SObject() :
		gc(GarbageCollection::GetInstance()),
		bPendingKill(false), bMark(false)
	{
	}
	SObject::SObject(const SObject& other) :
		gc(other.gc), 
		bPendingKill(other.bPendingKill.load(std::memory_order::memory_order_relaxed)),
		bMark(other.bMark)
	{
	}
	SObject::SObject(SObject&& other) noexcept :
		gc(other.gc),
		bPendingKill(other.bPendingKill.load(std::memory_order::memory_order_relaxed)),
		bMark(other.bMark)
	{
	}
	SObject::~SObject()
	{
		onDestroy.Notify(this);

		if (!bPendingKill.load(std::memory_order::memory_order_acquire))
			gc->RemoveObject(this);
	}

	auto SObject::operator new(std::size_t size) -> void*
	{
		SObject* ptr = static_cast<SObject*>(::operator new(size));
		GarbageCollection::GetInstance()->AddObject(static_cast<SObject*>(ptr));
		return ptr;
	}

	void SObject::operator delete(void* ptr, std::size_t size)
	{
		SObject* obj = static_cast<SObject*>(ptr);
		GarbageCollection::GetInstance()->DeleteObject(obj);
		::operator delete(obj);
	}

	auto SObject::IsPendingKill() const -> bool
	{
		return bPendingKill.load(std::memory_order::memory_order_acquire);
	}
	auto SObject::IsMark() const -> bool
	{
		return bMark;
	}

	void SObject::OnPropertyChanged(const reflection::Property& prop)
	{
	}

	void SObject::Destroy()
	{
		onDestroy.Notify(this);
		onDestroy.Clear();
		bPendingKill.store(true, std::memory_order::memory_order_release);
	}
}