#include "EntitySystem.h"
#include <LBase/typeindex.h>
#include <LFramework/Adaptor/LAdaptor.h>

using namespace ecs;

namespace ecs {
	namespace details {
		using namespace leo;

		template<typename _type1 = leo::uint16, typename _type2 = leo::uint16>
		struct SaltHandle {
			_type1 salt;
			_type2 index;

			lconstexpr SaltHandle(_type1 salt_, _type2 index_) lnoexcept
				:salt(salt_), index(index_)
			{}

			lconstexpr SaltHandle() lnoexcept
				: salt(0), index(0)
			{}


			lconstexpr bool operator==(const SaltHandle<_type1, _type2>& rhs) const lnoexcept
			{
				return salt == rhs.salt && index == rhs.index;
			}

			lconstexpr bool operator!=(const SaltHandle<_type1, _type2>& rhs) const lnoexcept
			{
				return !(*this == rhs);
			}

			// conversion to bool
			// e.g. if(id){ ..nil.. } else { ..valid or not valid.. }
			lconstexpr operator bool() const lnoexcept
			{
				return salt != 0 && index != 0;
			}
		};

		lconstexpr auto salt_default_size = 64 * 1024u;

		template<typename _type1 = leo::uint16, typename _type2 = leo::uint16, decltype(salt_default_size) size = salt_default_size - 3>
		class SaltHandleArray {
		public:
			using salt_type = _type1;
			using index_type = _type2;

			lconstexpr static index_type end_index = std::numeric_limits<_type2>::max();
			lconstexpr static index_type invalid_index = end_index - 1;

			static_assert(std::is_unsigned_v<_type2>);
			static_assert(size != std::numeric_limits<_type2>::max());
			static_assert(size != invalid_index);


			SaltHandleArray() {
				index_type i;
				for (i = size - 1; i > 1; --i)
				{
					buffer[i].salt = 0;
					buffer[i].next_index = i - 1;
				}
				buffer[1].salt = 0;
				buffer[1].next_index = end_index;     // end marker
				free_index = size - 1;

				// 0 is not used because it's nil
				buffer[0].salt = ~0;
				buffer[0].next_index = invalid_index;
			}

			SaltHandle<salt_type, index_type> RentDynamic() lnoexcept {
				if (free_index == end_index)
					return SaltHandle<salt_type, index_type>();

				SaltHandle<salt_type, index_type> ret{ buffer[free_index].salt,free_index };

				auto& element = buffer[free_index];

				free_index = element.next_index;

				element.next_index = invalid_index;

				return ret;
			}

			void Return(const SaltHandle<salt_type, index_type>& handle) lnoexcept {
				auto index = handle.index;
				LAssert(IsUsed(index), " Index was not used, Insert() wasn't called or Remove() called twice");

				auto& salt = buffer[index].salt;
				LAssert(handle.salt == salt, "Handle Is't Valid");
				++salt;

				buffer[index].next_index = free_index;
				free_index = index;
			}


			bool IsValid(const SaltHandle<salt_type, index_type>& handle) const lnoexcept
			{
				if (!handle)
					return false;

				if (handle.index > size)
					return false;

				return buffer[handle.index].salt == handle.salt;
			}

			bool IsUsed(index_type index) const lnoexcept {
				return buffer[index].next_index == invalid_index;
			}

		private:
			void Remove(index_type index) {
				if (free_index == index)
					free_index = buffer[index].next_index;
				else {
					auto old = free_list;
					auto it = buffer[old].next_index;

					for (;;) {
						auto next = buffer[it].next_index;

						if (it == index)
						{
							buffer[old].next_index = next;
							break;
						}

						old = it;
						it = next;
					}
				}
				buffer[index].next_index = invalid_index;
			}


			struct Element {
				salt_type salt;
				index_type next_index;
			};

			Element buffer[size];

			index_type free_index;

		};
	}

	details::SaltHandleArray<> SaltHandleArray;
	leo::unordered_map<EntityId, std::unique_ptr<Entity>> EntityMap;
	leo::unordered_multimap<leo::type_index, std::unique_ptr<System>> SystemMap;

	leo::uint16              IdToIndex(const EntityId id) { return id & 0xffff; }
	details::SaltHandle<>    IdToHandle(const EntityId id) { return details::SaltHandle<>(id >> 16, id & 0xffff); }
	EntityId            HandleToId(const details::SaltHandle<> id) { return (((leo::uint32)id.salt) << 16) | ((leo::uint32)id.index); }
}


leo::uint32 ecs::EntitySystem::Update(const UpdateParams & params)
{
	for (auto& pSystem : SystemMap)
	{
		pSystem.second->Update(params);
	}

	return {};
}

leo::observer_ptr<Entity> ecs::EntitySystem::Add(const leo::type_info& type_info, EntityId id, std::unique_ptr<Entity> pEntity)
{
	LAssert(SaltHandleArray.IsUsed(id), "SaltHandleArray Things go awry");
	LAssert(EntityMap.count(id) == 0, "EntitySystem Things go awry");
	return leo::make_observer(EntityMap.emplace(id, pEntity.release()).first->second.get());
}

EntityId EntitySystem::GenerateEntityId() const lnothrow {
	auto ret = InvalidEntityId;

	ret = HandleToId(SaltHandleArray.RentDynamic());

	return ret;
}

void EntitySystem::RemoveEntity(EntityId id) lnothrow {
	if (id == InvalidEntityId)
		return;
	LAssert(EntityMap.count(id) == 1, "Remove Same Entity Many Times");
	EntityMap.erase(id);
	SaltHandleArray.Return(IdToHandle(id));
}



EntitySystem & ecs::EntitySystem::Instance()
{
	static EntitySystem Instance;
	return Instance;
}

leo::observer_ptr<System> ecs::EntitySystem::Add(const leo::type_info& type_info, std::unique_ptr<System> pSystem)
{
	return leo::make_observer(SystemMap.emplace(leo::type_index(type_info), pSystem.release())->second.get());
}
