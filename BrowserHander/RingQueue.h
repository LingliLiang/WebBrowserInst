#pragma once

# ifndef _UNLOCK_Queue_h_
# define _UNLOCK_Queue_h_

#include <new>
#define NOMINMAX
#include <Windows.h>
#include <cassert>
#include <algorithm>
namespace uqueue // unlock queue
{
	namespace internal_lock
	{
		class SpinLockEx
		{
		public:

			SpinLockEx() : m_lock(0) {}
			~SpinLockEx() {}

			void Lock()
			{
				while (InterlockedCompareExchange(&m_lock, 1, 0) != 0)
				{
					Sleep(0);
				}
			}

			void Unlock()
			{
				InterlockedExchange(&m_lock, 0);
			}

		protected:
			volatile unsigned int m_lock;

		private:
			SpinLockEx(const SpinLockEx&) {}
			void operator=(const SpinLockEx&) {}
		};

		class Lock
		{
		public:
			Lock() 
			{
				::InitializeCriticalSectionAndSpinCount(&cs, 2000);
			}
			~Lock()
			{
				::DeleteCriticalSection(&cs);
			}

			bool Try()
			{
				if (::TryEnterCriticalSection(&cs) != FALSE) {
					return true;
				}
				return false;
			}

			// Take the lock, blocking until it is available if necessary.
			void Dolock()
			{
				::EnterCriticalSection(&cs);
			}

			// Release the lock.  This must only be called by the lock's holder: after
			// a successful call to Try, or a call to Lock.
			void Unlock()
			{
				::LeaveCriticalSection(&cs);
			}

		private:
			CRITICAL_SECTION cs;
		private:
			Lock(const Lock&) {}
			void operator=(const Lock&) {}
		};

		class AutoLock
		{
		public:
			explicit AutoLock(Lock& m) : m_(m)
			{
				m_.Dolock();
			}
			~AutoLock()
			{
				m_.Unlock();
			}

		private:
			Lock& m_;
		private:
			AutoLock(const AutoLock& l) :m_(l.m_){}
			void operator=(const AutoLock&) {}
		};
	}

	template<typename _Tp/*data type*/>
	class RingQueueBase 
	{
	public:

		//check size is power of 2
		inline bool is_power_of_two(unsigned int size)
		{
			return size != 0 && (size & (size - 1)) == 0;
		}

		inline void roundup_size(unsigned int& size)
		{
			if (!is_power_of_two(size))
			{
				unsigned int max_uint = (unsigned int)((unsigned int)~0);
				unsigned int and_size = ~(max_uint & (max_uint >> 1));
				while ((and_size & size) == 0)
				{
					and_size = and_size >> 1;
				}
				size = and_size << 1;
			}
		}

		//size- count of _Tp.
		//mcpy - use memcpy replace =
		//note: tatol byte of _Tp array,must less than MAX(unsigned int) range.
		RingQueueBase(unsigned int size, unsigned int mcpy = 0)
			:_front(0),
			_rear(0),
			_max_size(0),
			_memcpy_method(mcpy),
			data_(NULL)
		{
			//round up to the next power of 2
			if (!is_power_of_two(size))
			{
				roundup_size(size);
			}
			_max_size = size;
			_size_len = size_of();
			_max_size_len = _max_size*_size_len;
		}
		~RingQueueBase() {}


		virtual bool Initialize() 
		{
			clear();
			return true;
		}

		//size of _Tp
		virtual unsigned int size_of()
		{
			return sizeof(_Tp);
		}

		//get _Tp true data length, incase _Tp is struct with pointer.
		virtual unsigned int peer_length() { return _size_len; };

		//free _Tp data, incase _Tp is struct with pointer.
		virtual void peer_free(_Tp& value) {};

		//free something
		virtual void clear() { _rear = _front = 0; }

		_Tp& front() { return data_[_rear / _size_len]; }

		unsigned int push(const _Tp& value)
		{
			unsigned int len = 0;

			//use left buffer
			len = std::min(_size_len, _max_size_len - _front + _rear);

			// none left in buffer
			if (len == 0) return 0;

			//Ensure that we sample the _front index -before- we
			//start putting bytes into the queue.
			_front = *(&_front);

			if (_memcpy_method)
			{
				unsigned char* pvalue = reinterpret_cast<unsigned char*>(const_cast<_Tp*>(&value));
				unsigned char* pdata = reinterpret_cast<unsigned char*>(data_);
				unsigned int cpy_len = 0;
				//first put the data starting from _front to buffer end
				cpy_len = std::min(len, _max_size_len - (_front  & (_max_size_len - 1)));
				memcpy(pdata + (_front & (_max_size_len - 1)), pvalue, cpy_len);

				//then put the rest(if any) at the beginning of the buffer
				memcpy(pdata, pvalue + cpy_len, len - cpy_len);
			}
			else
			{
				data_[_front / _size_len] = value;
			}

			//Ensure that we add the bytes to the buffer -before-
			//we update the _front index.
			_front = *(&_front);

			_front += len;
			return len;
		}

		unsigned int pop(_Tp& value)
		{
			unsigned int len = 0;

			//have buffer length
			len = std::min(_size_len, _front - _rear);

			// none data in buffer
			if (len == 0) return 0;

			//Ensure that we sample the _front index -before- we
			//start putting bytes into the queue.
			_front = *(&_front);

			if (_memcpy_method)
			{
				unsigned char* pvalue = reinterpret_cast<unsigned char*>(&value);
				unsigned char* pdata = reinterpret_cast<unsigned char*>(data_);
				unsigned int cpy_len = 0;
				//first put the data starting from _rear to buffer end
				cpy_len = std::min(len, _max_size_len - (_rear  & (_max_size_len - 1)));
				memcpy(pvalue, pdata + (_rear & (_max_size_len - 1)),cpy_len);

				//then put the rest(if any) at the beginning of the buffer
				memcpy(pvalue + cpy_len, pdata, len - cpy_len);
			}
			else
			{
				value = data_[_rear / _size_len];
			}

			//Ensure that we add the bytes to the buffer -before-
			//we update the _front index.
			_rear = *(&_rear);

			_rear += len;
			return len;
		}

		unsigned int lock_push(const _Tp& value)
		{
			internal_lock::AutoLock lock(_lock_in);
			return push(value);
		}

		unsigned int lock_pop(const _Tp& value)
		{
			internal_lock::AutoLock lock(_lock_out);
			return pop(value);
		}

		bool full() const { return (_front - _rear) == _max_size_len; }
		bool empty() const { return (_front - _rear) == 0;}
		unsigned int  size() const { return (_front - _rear) / _size_len; }
		unsigned int  capacity() const { return _max_size; }
		unsigned int  capacity_of_len() const { return _max_size_len; }
		unsigned int data_length() { return _front - _rear; }
		_Tp*& data() { return data_; }
		void memcopy(bool t) { _memcpy_method = t ? 1 : 0; }
protected:
		volatile unsigned int _front;  // front index, push in
		volatile unsigned int _rear;   // rear index, pop out

		unsigned int _max_size; // queue capacity, power of two
		unsigned int _max_size_len; // tatol byte of _Tp array
		unsigned int _size_len; // byte of _Tp size
		unsigned int _memcpy_method; //use memcpy instead of =

		internal_lock::Lock _lock_in; // lock for multi-thread
		internal_lock::Lock _lock_out; // lock for multi-thread

		_Tp* data_;   // data record occupy symbol
		//_Tp data_[0];
	};

	template<typename _Tp>
	struct AllocateBase {
		void* allocate(unsigned int msize)
		{
			char *p = new char[msize];
			if (p) memset(p, 0, msize);
			return p;
		}

		void free(void *p)
		{
			delete[](char *)p;
		}
	};

	template<typename _Tp,
		typename Queue_ = RingQueueBase<_Tp>,
		typename Alloc_ = AllocateBase<_Tp> >
	class RingQueue {
	public:
		typedef _Tp  value_type;

		RingQueue(int msize) : impl(msize)
		{
			value_type*& data = impl.data();
			data = reinterpret_cast<_Tp*>(alloc.allocate(impl.capacity_of_len()));
			assert(data);
		}
		~RingQueue()
		{
			alloc.free((void*)impl.data());
		}

		value_type& front() { return impl.front(); }
		const value_type& front() const { return impl.front(); }

		unsigned int push(const value_type& value) { return impl.push(value); }
		unsigned int pop(value_type& value) { return impl.pop(value); }
		unsigned int lock_push(const value_type& value) { return impl.lock_push(value); }
		unsigned int lock_pop(value_type& value) { return impl.lock_pop(value); }

		void memcopy(bool cpy) { impl.memcopy()	}
		bool full() const { return impl.full(); }
		bool empty() const { return impl.empty(); }
		int  size() const { return impl.size(); }
		unsigned int  capacity() const { return impl.capacity(); }
	private:
		Alloc_ alloc;
		Queue_ impl;
	};

} // namespace uqueue

# endif  // _UNLOCK_Queue_h_

