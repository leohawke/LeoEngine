#include "EntitySystem.h"
#include <LFramework/Adaptor/LAdaptor.h>

using namespace ecs;

namespace ecs {
	namespace details {
		using namespace leo;

		unordered_map<EntityId, std::unique_ptr<Entity>>& EntityBuffer() {
			static unordered_map<EntityId, std::unique_ptr<Entity>> Instance;
			return Instance;
		}

		vector<std::unique_ptr<System>>& SystemBuffer() {
			static vector<std::unique_ptr<System>> Instance;
			return Instance;
		}

		template<typename _type1 = leo::uint16,typename _type2 = leo::uint16>
		struct SaltHandle {
			_type1 salt;
			_type2 index;

			lconstexpr SaltHandle(_type1 salt_,_type2 index_) lnoexcept
				:salt(salt_),index(index_)
			{}

			lconstexpr SaltHandle() lnoexcept
				:salt(0),index(0)
			{}


			lconstexpr bool operator==(const SaltHandle<_type1, _type2>& rhs) const lnoexcept
			{
				return m_Salt == rhs.m_Salt && m_Index == rhs.m_Index;
			}

			lconstexpr bool operator!=(const SaltHandle<_type1, _type2>& rhs) const lnoexcept
			{
				return !(*this == rhs);
			}

			static lconstexpr SaltHandle  nil = {};

			// conversion to bool
			// e.g. if(id){ ..nil.. } else { ..valid or not valid.. }
			lconstexpr operator bool() const lnoexcept
			{
				return *this != nil;
			}
		};

		lconstexpr auto salt_default_size = 64 * 1024u;

		template<typename _type1 = leo::uint16, typename _type2 = leo::uint16,decltype(salt_default_size) size = salt_default_size-3>
		class SaltHandleArray {
			using salt_type = _type1;
			using index_type = _type2;

			lconstexpr static index_type end_index = std::numeric_limits<_type2>::max();
			lconstexpr static index_type valid_index = end_index - 1;

			static_assert(std::is_unsigned_v<_type2>);
			static_assert(size != std::numeric_limits<_type2>::max());
			static_assert(size != valid_index);


			SaltHandle<salt_type, index_type> RentDynamic() lnoexcept {
				if (free_index == end_index)
					return SaltHandle<salt_type, index_type>::nil;

				SaltHandle<salt_type, index_type> ret{ buffer[free_index].salt,free_index };

				auto& element = buffer[free_index];
				
				free_index = element.next_index;

				element.next_index = valid_index;

				return ret;
			}


			void Return(const SaltHandle<salt_type, index_type>& handle) lnoexcept {
				auto index = handle.index;
				LAssert(IsUsed(index)," Index was not used, Insert() wasn't called or Remove() called twice");

				auto& salt = buffer[index].salt;
				LAssert(handle.salt == salt,"Handle Is't Valid");
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
				return buffer[index].next_index == valid_index;
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
				buffer[index].next_index = valid_index;
			}


			struct Element {
				salt_type salt;
				index_type next_index;
			};

			Element buffer[size];

			index_type free_index;

		};

	}
}


leo::uint32 ecs::EntitySystem::Update(const UpdateParams & params)
{
	//TODO Coroutine;
	for(auto& pSystem : details::SystemBuffer())
	{
		pSystem->Update(params);
	}

	return {};
}

leo::observer_ptr<Entity> ecs::EntitySystem::SpawnEntity(const EntitySystemSpawnEntityParams & params)
{
	if (params.id != InvalidEntityId) {
		if (details::EntityBuffer().find(params.id) != details::EntityBuffer().end()) {
			details::EntityBuffer()[params.id]->ReSpawn(params.params);
		}
	}
	return leo::observer_ptr<Entity>();
}

leo::observer_ptr<System> ecs::EntitySystem::Add(std::unique_ptr<System> pSystem)
{
	return leo::observer_ptr<System>();
}
