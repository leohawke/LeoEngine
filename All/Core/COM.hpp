////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2015.
// -------------------------------------------------------------------------
//  File name:   Core/COM.hpp
//  Version:     v1.00
//  Created:     05/16/2015 by leo hawke.
//  Compilers:   Visual Studio.NET 2015
//  Description: 一些COM相关的接口
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#ifndef Core_CoreCOM_HPP
#define Core_CoreCOM_HPP

#include "coredebug.hpp"
#include <utility.hpp>
namespace leo
{
	namespace win
	{
		namespace details {
			template<typename COM,typename = decltype(&COM::SetPrivateData)>
			void Print(COM* &com, decltype(&COM::SetPrivateData)) {
				UINT DataSize;
				com->GetPrivateData(::WKPDID_D3DDebugObjectName, &DataSize, nullptr);
				if (DataSize != 0) {
					std::string name;
					name.resize(DataSize);
					com->GetPrivateData(::WKPDID_D3DDebugObjectName, &DataSize, &name[0]);
					if (DataSize > 1 && (name[1] == char())) {
						std::wstring wname;
						wname.resize(DataSize / 2);
						com->GetPrivateData(::WKPDID_D3DDebugObjectName, &DataSize, &wname[0]);
						DebugPrintf(L"%s address: %p,refcount: ", wname.c_str(), com);
					}
					else
						DebugPrintf("%s address: %p,refcount: ", name.c_str(), com);
					return;
				}
				DebugPrintf("UNKNOWN address: %p,refcount: ", com);
			}

			template<typename COM>
			void Print(COM* &com,...) {
				DebugPrintf("UNKNOWN address: %p,refcount: ", com);
			}

			template<typename COM>
			void PrintAndRelease(COM* &com) {
				Print(com,nullptr);
				DebugPrintf("%u\n", com->Release());
			}
		}

		template<typename COM>
		void ReleaseCOM(COM* &com)
		{
			if (com) {
				details::PrintAndRelease(com);
			}
			com = nullptr;
		}

		namespace details {
			template<typename COM>
			struct com_deleter {
				void operator()(COM* obj) {
					obj?obj->Release():0;
				}
			};
		}

		

		template<class COM,class Dx>
		class unique_com_base
		{	// stores pointer and deleter
		public:
			typedef typename std::remove_reference<Dx>::type Dx_noref;
			typedef COM* pointer;

			unique_com_base(pointer Ptr, Dx Dt)
				: Myptr(Ptr), Mydel(Dt)
			{	// construct with pointer and deleter
			}

			template<class Ptr2,
			class Dx2>
				unique_com_base(Ptr2 Ptr, Dx2 Dt)
				: Myptr(Ptr), Mydel(Dt)
			{	// construct with compatible pointer and deleter
			}

			Dx_noref& get_deleter()
			{	// return reference to deleter
				return (Mydel);
			}

			const Dx_noref& get_deleter() const
			{	// return const reference to deleter
				return (Mydel);
			}

			pointer Myptr;	// the managed pointer
			Dx Mydel;		// the deleter
		};

		template<class COM,
		class Dx = details::com_deleter<COM >>	// = details::com_deleter<COM>
		class unique_com
			: public unique_com_base<COM, Dx>
		{	// non-copyable pointer to an object
		public:
			typedef unique_com<COM, Dx> Myt;
			typedef unique_com_base<COM, Dx> Mybase;
			typedef typename Mybase::pointer pointer;
			typedef COM element_type;
			typedef Dx deleter_type;

			unique_com()
				: Mybase(pointer(), Dx())
			{	// default construct
				static_assert(!is_pointer<Dx>::value,
					"unique_com constructed with null deleter pointer");
			}

			unique_com(leo::nullptr_t)
				: Mybase(pointer(), Dx())
			{	// null pointer construct
				static_assert(!is_pointer<Dx>::value,
					"unique_com constructed with null deleter pointer");
			}

			Myt& operator=(leo::nullptr_t)
			{	// assign a null pointer
				reset();
				return (*this);
			}

			explicit unique_com(pointer Ptr)
				: Mybase(Ptr, Dx())
			{	// construct with pointer
				static_assert(!is_pointer<Dx>::value,
					"unique_com constructed with null deleter pointer");
			}

			unique_com(pointer Ptr,
				std::conditional_t<std::is_reference<Dx>::value, Dx,
				const typename std::remove_reference<Dx>::type&> Dt)
				: Mybase(Ptr, Dt)
			{	// construct with pointer and (maybe const) deleter&
			}

			unique_com(pointer Ptr, typename std::remove_reference<Dx>::type&& Dt)
				: Mybase(Ptr, std::move(Dt))
			{	// construct by moving deleter
				//		static_assert(!tr1::is_reference<Dx>::value,
				//			"unique_com constructed with reference to rvalue deleter");
			}

			unique_com(unique_com&& Right)
				: Mybase(Right.release(),
					std::forward<Dx>(Right.get_deleter()))
			{	// construct by moving Right
			}

			template<class COM2,
			class Dx2>
				unique_com(unique_com<COM2, Dx2>&& Right)
				: Mybase(Right.release(),
					std::forward<Dx2>(Right.get_deleter()))
			{	// construct by moving Right
			}

			template<class COM2,
			class Dx2>
				Myt& operator=(unique_com<COM2, Dx2>&& Right)
			{	// assign by moving Right
				reset(Right.release());
				this->get_deleter() = std::move(Right.get_deleter());
				return (*this);
			}

			Myt& operator=(Myt&& Right)
			{	// assign by moving Right
				if (this != std::addressof(Right))
				{	// different, do the move
					reset(Right.release());
					this->get_deleter() = std::move(Right.get_deleter());
				}
				return (*this);
			}

			void swap(Myt&& Right)
			{	// swap elements
				if (this != std::addressof(Right))
				{	// different, do the swap
					std::swap(this->Myptr, Right.Myptr);
					std::swap(this->get_deleter(),
						Right.get_deleter());
				}
			}

			void swap(Myt& Right)
			{	// swap elements
				std::swap(this->Myptr, Right.Myptr);
				std::swap(this->get_deleter(),
					Right.get_deleter());
			}

			~unique_com()
			{	// destroy the object
				Delete();
			}


			pointer operator->() const
			{	// return pointer to class object
				return get();
			}

			decltype(auto) operator&(){
				return &(this->Myptr);
			}

			decltype(auto) operator&() const{
				return &(this->Myptr);
			}

			pointer get() const
			{	// return pointer to object
				return (this->Myptr);
			}

			operator bool() const
			{	// test for non-null pointer
				return (this->Myptr != pointer() ? true : false);
			}

			template<typename base_com_pointer, typename = std::enable_if_t<std::is_base_of<leo::remove_pointer_t<base_com_pointer>, COM>::value, void>>
			operator base_com_pointer() const {
				return get();
			}

			pointer release()
			{	// yield ownership of pointer
				pointer _Ans = this->Myptr;
				this->Myptr = pointer();
				return (_Ans);
			}

			void reset(pointer Ptr = pointer())
			{	// establish new pointer
				if (Ptr != this->Myptr)
				{	// different pointer, delete old and reassign
					Delete();
					this->Myptr = Ptr;
				}
			}

		private:
			void Delete()
			{	// delete the pointer
				if (this->Myptr != pointer())
					this->get_deleter()(this->Myptr);
				this->Myptr = pointer();
			}

			unique_com(const Myt&);	// not defined
			template<class COM2,
			class Dx2>
				unique_com(const unique_com<COM2, Dx2>&);	// not defined

			Myt& operator=(const Myt&);	// not defined
			template<class COM2,
			class Dx2>
				Myt& operator=(const unique_com<COM2, Dx2>&);	// not defined
		};

		template<typename COM>
		decltype(auto) make_scope_com(COM&& obj) {
			return unique_com<leo::remove_pointer_t<COM>>(lforward(obj));
		}

		template<typename COM>
		decltype(auto) make_scope_com(std::nullptr_t = nullptr) {
			return unique_com<COM>(nullptr);
		}
	}
}

#endif