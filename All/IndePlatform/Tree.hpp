//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   IndePlatform/Tree.hpp
//  Created:     02/06/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 树结构的实现
// -------------------------------------------------------------------------
//  History:
//		
//
////////////////////////////////////////////////////////////////////////////

#ifndef Indeplatform_Tree_hpp
#define Indeplatform_Tree_hpp

#include "utility.hpp"
#include "memory.hpp"
#include "LeoMath.h"
#include <vector>
namespace leo{

	template<typename ValueType, typename Alloc = std::allocator<ValueType>>
	class QuadTree
	{
		struct Node
		{
			//x,y => Corner
			//z,w => Size
			float4 mRect;
			std::pair<ValueType*, float2> mLeafValues[four];
			Node* mChildNodes[four];

			Node()
			{
				std::memset(&mRect, 0, sizeof(mRect));
				std::fill_n(mLeafValues, four, nullptr);
				std::fill_n(mChildNodes, four, nullptr);
			}

			Node(const float4& rect)
				:mRect(rect)
			{
				std::fill_n(mLeafValues, four,std::make_pair(nullptr, float2()));
				std::fill_n(mChildNodes, four, nullptr);
			}
		};
	public:
		using value_type = ValueType;
		using node_type = Node;
		using value_alloc = Alloc;
		using node_alloc = typename value_alloc::template rebind<node_type>::other;


		bool InRect(const float4& rect, const float2& pos) const
		{
			if (pos.x < rect.x - rect.z / 2 || pos.x > rect.x + rect.z / 2 || pos.y < rect.y - rect.w / 2 || pos.y > rect.y + rect.w / 2)
				return false;
			return true;
		}

		std::uint8_t CalcDirection(const float4& rect, const float2& pos) const
		{
			if (!InRect(rect, pos))
				throw std::out_of_range("This Object Didn't Inside In Quad");
			std::uint8_t dire = (uint8)(pos.x < rect.x);
			dire <<= 1;
			dire |= (uint8)(pos.y < rect.y);
			return dire;
		}

		float4 CalcRect(const float4& rect, std::uint8_t dire) const
		{
			float4 result;
			result.z = rect.z / 2;
			result.w = rect.w / 2;
			switch (dire)
			{
			case 0:
				result.x = rect.x + result.z / 2;
				result.y = rect.y + result.w / 2;
				break;
			case 1:
				result.x = rect.x + result.z / 2;
				result.y = rect.y - result.w / 2;
				break;
			case 2:
				result.x = rect.x - result.z / 2;
				result.y = rect.y + result.w / 2;
				break;
			case 3:
				result.x = rect.x - result.z / 2;
				result.y = rect.y - result.w / 2;
				break;
			}
			return result;
		}

		void NodeInsert(Node* node, const value_type& value, const float2& pos)
		{
			std::uint8_t index = -1;
			try{
				index = CalcDirection(node->mRect, pos);
			}
			catch (std::out_of_range& ex){
				ex.what();
				return;
			}

			//没有值也没有子节点
			if (node->mLeafValues[index].first == nullptr && node->mChildNodes[index] == nullptr)
			{
				node->mLeafValues[index].first = mValueAlloc.allocate(1);
				mValueAlloc.construct(node->mLeafValues[index].first, value);
				node->mLeafValues[index].second = pos;
			}
			else{
				auto has_value_no_child = node->mLeafValues[index].first != nullptr && node->mChildNodes[index] == nullptr;
				//有值没有子节点
				if (has_value_no_child){
					node->mChildNodes[index] = mNodeAlloc.allocate(1);
					mNodeAlloc.construct(node->mChildNodes[index], CalcRect(node->mRect, index));
					NodeInsert(node->mChildNodes[index], std::move(*node->mLeafValues[index].first), node->mLeafValues[index].second);
					//已经move,只需要释放内存
					//警告:in VC++14 CTP ,the second parmter will be check
					mValueAlloc.deallocate(node->mLeafValues[index].first, 1);
					node->mLeafValues[index].first = nullptr;
				}
				NodeInsert(node->mChildNodes[index], value, pos);
			}
		}

		void NodeInsert(Node* node,value_type&& value, const float2& pos)
		{
			std::uint8_t index = -1;
			try{
				index = CalcDirection(node->mRect, pos);
			}
			catch (std::out_of_range& ex){
				ex.what();
				return;
			}

			//没有值也没有子节点
			if (node->mLeafValues[index].first == nullptr && node->mChildNodes[index] == nullptr)
			{
				node->mLeafValues[index].first = mValueAlloc.allocate(1);
				mValueAlloc.construct(node->mLeafValues[index].first, std::move(value));
				node->mLeafValues[index].second = pos;
			}
			else{
				auto has_value_no_child = node->mLeafValues[index].first != nullptr && node->mChildNodes[index] == nullptr;
				//有值没有子节点
				if (has_value_no_child){
					node->mChildNodes[index] = mNodeAlloc.allocate(1);
					mNodeAlloc.construct(node->mChildNodes[index], CalcRect(node->mRect, index));
					NodeInsert(node->mChildNodes[index], std::move(*node->mLeafValues[index].first), node->mLeafValues[index].second);
					//已经move,只需要释放内存
					//警告:in VC++14 CTP ,the second parmter will be check
					mValueAlloc.deallocate(node->mLeafValues[index].first, 1);
					node->mLeafValues[index].first = nullptr;
				}
				NodeInsert(node->mChildNodes[index], std::move(value), pos);
			}
		}

		void ValueDelete(ValueType* value)
		{
			if (value == nullptr)
				return;
			mValueAlloc.destroy(value);
			mValueAlloc.deallocate(value, 1);
		}

		void NodeDelete(Node* node)
		{
			if (node == nullptr)
				return;
			for (auto childnode : node->mChildNodes)
				NodeDelete(childnode);
			for (auto leafvalue : node->mLeafValues)
				ValueDelete(leafvalue.first);
			mNodeAlloc.destroy(node);
			mNodeAlloc.deallocate(node, 1);
		}

		
	private:
		Node * mRootNode = nullptr;
		value_alloc mValueAlloc;
		node_alloc mNodeAlloc;
	public:
		QuadTree(const float4& rect)
		{
			mRootNode = mNodeAlloc.allocate(1);
			mNodeAlloc.construct(mRootNode, rect);
		}

		~QuadTree()
		{
			NodeDelete(mRootNode);
		}
		void Insert(const value_type& value, const float2& pos)
		{
			NodeInsert(mRootNode, value, pos);
		}
	};

}
#endif