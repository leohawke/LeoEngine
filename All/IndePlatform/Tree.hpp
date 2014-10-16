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
#include <functional>
#include <stack>
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

		bool IsEmpty(Node * node)
		{
			if (node == nullptr)
				return true;
			for (auto leafvalue : node->mLeafValues){
				if (leafvalue.first != nullptr)
					return false;
			}
			for (auto childnode : node->mChildNodes){
				if (!IsEmpty(childnode))
					return false;
			}
			return true;
		}

		bool FindNodeAndDelete(Node* node,const value_type& value)
		{
			if (node == nullptr)
				return false;
			//值查找
			bool finded = false;
			for (auto &leafvalue : node->mLeafValues){
				if (!finded && (leafvalue.first != nullptr) && *leafvalue.first == value){
					mValueAlloc.destroy(leafvalue.first);
					mValueAlloc.deallocate(leafvalue.first, 1);
					leafvalue.first = nullptr;
					finded = true;
				}
			}
			for (auto & childnode : node->mChildNodes)
				if (!finded)
				finded = FindNodeAndDelete(childnode,value);
				
			for (auto & childnode : node->mChildNodes){
				if (childnode != nullptr && IsEmpty(childnode)){
					mNodeAlloc.destroy(childnode);
					mNodeAlloc.deallocate(childnode, 1);
					childnode = nullptr;
				}
			}
			return finded;
		}
	private:
		Node * mRootNode = nullptr;
		value_alloc mValueAlloc;
		node_alloc mNodeAlloc;


		template<typename F,bool = std::is_function<F>::value>
		struct value_function {
			enum : bool {
				value = false
			};
		};

		template<typename RET,typename... Opt>
		struct value_function<RET(const value_type&,Opt...),true>{
			enum : bool{
				value = true
			};
		};

		template<typename RET,class C,typename... Opt>
		struct value_function < RET(C::*)(const value_type&, Opt...), false > {
			enum : bool {
				value = true
			};
		};

		template<typename RET, class C, typename... Opt>
		struct value_function < RET(C::*)(const value_type&, Opt...) const, false > {
			enum : bool {
				value = true
			};
		};

		template<class Fr>
		struct value_function<Fr, false> : value_function < decltype(&Fr::operator()) >
		{};
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

		void Erase(const value_type& value)
		{
			FindNodeAndDelete(mRootNode, value);
		}

		template<typename F,typename... OptPara>
		typename std::enable_if<value_function<typename std::remove_reference<F>::type>::value>::type
		Iterator(const F& f, OptPara&&... args)
		{
			std::stack<Node*> mNodeStack;

			mNodeStack.push(mRootNode);

			while (!mNodeStack.empty()){
				auto & node = mNodeStack.top();
				mNodeStack.pop();

				for (auto & leafvalue : node->mLeafValues){
					if (leafvalue.first != nullptr)
						f(*leafvalue.first, std::forward<OptPara>(args)...);
				}

				for (auto childnode : node->mChildNodes){
					childnode != nullptr? mNodeStack.push(childnode) : 0;
				}
			}
		}
	};

}
#endif