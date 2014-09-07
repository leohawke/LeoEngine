#include "Input.h"
#include <set>

namespace leo
{
	namespace win
	{
		static std::set<KeyAction*> Register;

		KeyAction::KeyAction()
		{
			Register.insert(this);
		}

		KeyAction::~KeyAction()
		{
			Register.erase(this);
		}

		KeysState::KeyState KeysState::m_currstates[256];
		KeysState::KeyState KeysState::m_prevstates[256];
		//这个函数有问题,不管了
		void KeysState::Update()
		{
			BYTE keysstate[256];
			GetKeyboardState(keysstate);
			for (UINT key = 0; key < 256; ++key)
			{
				//auto keystate = GetAsyncKeyState(key);
				if (keysstate[key] & 0x80)
					m_currstates[key] = press_mask;
				else
					m_currstates[key] = 0;
				auto i = key;
				auto flagchange = m_currstates[i] ^ m_prevstates[i];
				auto flagrising = ~m_currstates[i] & flagchange;
				auto flagfalling = m_currstates[i] & flagchange;
				if (flagrising & press_mask)
					m_currstates[i] |= risingedge_mask;
				else
					m_currstates[i] &= (~risingedge_mask);
				if (flagfalling & press_mask)
					m_currstates[i] |= fallingedge_mask;
				else
					m_currstates[i] &= (~fallingedge_mask);
				m_prevstates[i] = m_currstates[i];
			}


			for (auto& ka : Register)
			{
				ka->Update();
			}
			//static unsigned long count = 0;
			//if (m_currstates['T'] & fallingedge_mask)
			//{
			//count++;
			//DebugPrintf(L"T的下降沿 %u\n", count);
			//}
			//if (m_currstates['T'] & risingedge_mask)
			//DebugPrintf(L"T的上升沿 %u\n", count);
			//DebugPrintf(L"更新计数 %u\n", count);
		}
#pragma warning(push)
#pragma warning(disable : 4800)
		bool KeysState::KeyRisingEdge(std::uint8_t key)
		{
			bool flag = m_currstates[key] & risingedge_mask;
			//if (flag)
			//m_currstates[key] &= (~risingedge_mask);
			return flag;
		}
		bool KeysState::KeyFallingEdge(std::uint8_t key)
		{
			bool flag = m_currstates[key] & fallingedge_mask;
			//if (flag)
			//m_currstates[key] &= (~fallingedge_mask);
			return flag;
		}
		bool KeysState::KeyPress(std::uint8_t key)
		{
			return m_currstates[key] & press_mask;
		}


#pragma warning(pop)
	}
}