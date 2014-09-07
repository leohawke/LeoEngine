#pragma once
#include "IndePlatform\clock.hpp"
#include "IndePlatform\Singleton.hpp"
#include "Mgr.hpp"
#include <thread>
namespace leo
{
	namespace win
	{
		class LB_API KeysState : leo::Singleton<KeysState>
		{
		public:
			using KeyState = std::uint8_t;
			~KeysState() = default;
		public:
			void Update();
			bool KeyRisingEdge(std::uint8_t key);
			bool KeyFallingEdge(std::uint8_t key);
			bool KeyPress(std::uint8_t key);
		private:
			// 7 6 5 4 3     2      1      0
			//             按下  下降沿  上升沿
			static const std::uint8_t risingedge_mask = 0x1;
			static const std::uint8_t fallingedge_mask = risingedge_mask << 1;
			static const std::uint8_t press_mask = fallingedge_mask << 1;
		private:
			static KeyState m_currstates[256];
			static KeyState m_prevstates[256];
		public:
			static KeysState* GetInstance()
			{
				return &global::globalKeysState;
			}
		};

		class KeyAction
		{
		protected:
			KeyAction();
			~KeyAction();
		protected:
			virtual void Update() = 0
			{
				assert(0);
			}
			friend class KeysState;
		};

		inline bool KeyFallingEdge(std::uint8_t key)
		{
			return leo::global::globalKeysState.KeyFallingEdge(key);
			//return GetAsyncKeyState(key) == 0x8000;
		}
		inline bool KeyRisingEdge(std::uint8_t key)
		{
			return leo::global::globalKeysState.KeyRisingEdge(key);
			//return GetAsyncKeyState(key) == 0x1;
		}
		inline bool KeyPress(std::uint8_t key)
		{
			return leo::global::globalKeysState.KeyPress(key);
			//return GetAsyncKeyState(key) & 0x8000;
		}

		//快速击打某按键(持续)
		class KeyTapDetector : public KeyAction
		{
			std::uint8_t m_key;
			bool m_valid = false;
			float m_dtMax;//单位,秒
			Clock::time_point m_tLast;//上次下降沿来临时的时间点
		public:
			KeyTapDetector(std::uint8_t key, float dtMax) :
				m_key(key), m_dtMax(dtMax), m_tLast(Clock::Now())
			{
				m_tLast += Clock::ToDuration(-dtMax);
			}
			//随时调用获取是否作出手势
			bool GetValid() const
			{
				return m_valid;
			}
		protected:
			//每帧调用
			void Update()
			{
				if (win::KeyFallingEdge(m_key))
				{
					auto curr = Clock::Now();
					auto dt = Clock::DurationTo(curr - m_tLast);
					m_tLast = curr;
					if (dt < m_dtMax)
						m_valid = true;
				}
				else
					m_valid = false;
			}
		};

		//连续击打某些按键(触发)
		class KeySequenceDetector : public KeyAction
		{
			std::unique_ptr<std::uint8_t[]> m_keys;
			uint8 m_count;
			float m_dtMax;
			std::function<void()> m_callback;
			uint8 m_curr;
			Clock::time_point m_start;
		public:
			KeySequenceDetector(std::initializer_list<std::uint8_t> keys, float dtmax, std::function<void()> callback)
				:m_keys(std::make_unique<std::uint8_t[]>(keys.size())), m_count(static_cast<uint8>(keys.size())), m_dtMax(dtmax), m_callback(callback), m_curr(0), m_start(Clock::Now())
			{
				std::copy(keys.begin(), keys.end(), m_keys.get());
			}
		protected:
			void Update()
			{
				for (std::uint8_t key = 'A'; key <= 'Z'; ++key)
				{
					if (key != m_keys[m_curr])
						if (win::KeyFallingEdge(key))
						{
						m_curr = 0;
						return;
						}
				}
				if (win::KeyFallingEdge(m_keys[m_curr]))
				{
					if (m_curr == 0)
					{
						m_start = Clock::Now();
						++m_curr;
						return;
					}

					float dt = Clock::DurationTo(Clock::Now() - m_start);
					if (dt < m_dtMax)
					{
						++m_curr;
						if (m_curr == m_count)
						{
							std::thread t(m_callback);
							t.detach();
							m_curr = 0;
						}
					}
					else
						m_curr = 0;
				}
				assert(m_curr < m_count);
			}
		};

		//按下某键不放(持续)
		class KeyHoldDetector : public KeyAction
		{
			std::uint8_t m_key;
			bool m_valid = false;
			bool m_flag = false;
			float m_dtMin;
			Clock::time_point m_tLast;//上次按下键的时间点
		public:
			KeyHoldDetector(std::uint8_t key, float dtmin = 1.f)
				:m_key(key), m_dtMin(dtmin), m_tLast(Clock::Now())
			{}

			bool GetValid() const
			{
				return m_valid;
			}
		protected:
			void Update()
			{
				auto curr = Clock::Now();
				if (leo::win::KeyRisingEdge(m_key))
				{
					m_flag = false;
					return;
				}
				if (leo::win::KeyFallingEdge(m_key))
				{
					m_tLast = curr;
					m_flag = true;
					return;
				}
				if (m_flag)
				{
					auto dt = Clock::DurationTo(curr - m_tLast);
					if (dt > m_dtMin)
					{
						m_tLast = curr;
						m_valid = true;
					}
					else
						m_valid = false;
				}
			}
		};

		//从按下某键到释放达到一定时间(触发)
		class KeySeqHoldDetector : public KeyAction
		{
			std::uint8_t m_key;
			float m_dtMin;
			std::function<void()> m_callback;
			Clock::time_point m_tLast;//下降沿的时间点
		public:
			KeySeqHoldDetector(std::uint8_t key, float dtmin, std::function<void()> callback)
				:m_key(key), m_dtMin(dtmin), m_callback(callback), m_tLast(Clock::Now())
			{
			}
		protected:
			void Update()
			{
				auto curr = Clock::Now();
				auto dt = m_dtMin;
				if (leo::win::KeyRisingEdge(m_key))
				{
					dt = Clock::DurationTo(curr - m_tLast);
					if (dt > m_dtMin)
					{
						std::thread t(m_callback);
						t.detach();
					}
				}
				if (leo::win::KeyFallingEdge(m_key))
					m_tLast = curr;
			}
		};
	}
	
}