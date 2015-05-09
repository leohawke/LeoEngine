#pragma once

#include <chrono>
#include <cstdint>
namespace leo
{
	class std_timer
	{
	public:
		std_timer()
		{
			start = std::chrono::steady_clock::now();
		}
		virtual ~std_timer()
		{}
		void update()
		{
			duration newelapse = std::chrono::steady_clock::now() - start;
			delta = newelapse - elapse;
			elapse = newelapse;
		}
		template<typename Return_Type, typename Ratio_Type = std::milli>
		Return_Type get_elapse() const noexcept
		{
			std::chrono::duration<duration::rep, Ratio_Type> result = 
			std::chrono::duration_cast<decltype(result)>(elapse);
			return static_cast<Return_Type>(result.count());
		}
		template<typename Return_Type, typename Ratio_Type = std::milli>
		Return_Type get_delta() const noexcept
		{
			std::chrono::duration<duration::rep, Ratio_Type> result =
			std::chrono::duration_cast<decltype(result)>(delta);
			return static_cast<Return_Type>(delta.count());
		}

	protected:
		using time_point = std::chrono::steady_clock::time_point;
		using duration = time_point::duration;
		time_point start;
		duration elapse;
		duration delta;
	};
	//FrankHB:污染环境
	//不再使用_
	template<typename PRECISION>
	class std_gametimer : public std_timer
	{
	public:
		std_gametimer()
			:std_timer(), stopflag(false), stopedelapse()
		{}
		~std_gametimer()
		{}
		void start()
		{
			if (!stopflag)
				return;
			stopflag = !stopflag;
			this->update();
			PRECISION newstopedelapse = get_elapse<PRECISION, std::chrono::seconds>();
			stopcount += this->get_delta<PRECISION,std::chrono::seconds>();
			stopedelapse = newstopedelapse;
		}
		void stop()
		{
			if (stopflag)
				return;
			this->update();
			stopedelapse = get_elapse<PRECISION, std::chrono::seconds>();
		}
		PRECISION getelapse()
		{
			if (stopflag)
				return stopedelapse -stopcount;
			this->update();
			return get_elapse<PRECISION, std::chrono::seconds>() - stopcount;
		}
 	private:
		bool stopflag;
		PRECISION stopedelapse;
		PRECISION stopcount;
	protected:
	};
	namespace win
	{
		class timer
		{
		public:
			timer();
			virtual ~timer()
			{}

			void update();

			template<typename _Return_type,typename _Ratio_type = std::milli>
			_Return_type get_elapse() const noexcept
			{
				const std::intmax_t den = typename std::ratio_divide<std::milli, _Ratio_type>::type::den;
				const std::intmax_t num = typename std::ratio_divide<std::milli, _Ratio_type>::type::num;
				return static_cast<_Return_type>(elapse*num/den);
			}
			template<typename _Return_type, typename _Ratio_type = std::milli>
			_Return_type get_delta() const noexcept
			{
				const std::intmax_t den = typename std::ratio_divide<std::milli, _Ratio_type>::type::den;
				const std::intmax_t num = typename std::ratio_divide<std::milli, _Ratio_type>::type::num;
				return static_cast<_Return_type>(delta*num/den);
			}
		private:
			std::int64_t start;
			std::int64_t freq;
			long double elapse;
			long double delta;
		};
		template<typename PRECISION>
		class gametimer : public timer
		{
		public:
			gametimer()
				:timer(), stopflag(false), stopedelapse()
			{}
			~gametimer()
			{}
			void start()
			{
				if (!stopflag)
					return;
				stopflag = !stopflag;
				this->update();
				PRECISION newstopedelapse = get_elapse<PRECISION, std::chrono::seconds>();
				stopcount += this->get_delta<PRECISION, std::chrono::seconds>();
				stopedelapse = newstopedelapse;
			}
			void stop()
			{
				if (stopflag)
					return;
				this->update();
				stopedelapse = get_elapse<PRECISION, std::chrono::seconds>();
			}
			PRECISION getelapse()
			{
				if (stopflag)
					return stopedelapse - stopcount;
				this->update();
				return get_elapse<PRECISION, std::chrono::seconds>() - stopcount;
			}
		private:
			bool stopflag;
			PRECISION stopedelapse;
			PRECISION stopcount;
		protected:
		};
	}
}