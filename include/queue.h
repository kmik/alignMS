#pragma once

#include <mutex>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
struct metaItem{

    std::string dst, src;

};
template<typename T>
class blocking_queue
{
public:
	blocking_queue(unsigned int size)
	: m_size(size), m_pushIndex(0), m_popIndex(0), m_count(0),
	m_data((T*)operator new(size * sizeof(T))),
	m_openSlots(size), m_fullSlots(0) {}

	blocking_queue(const blocking_queue&) = delete;
	blocking_queue(blocking_queue&&) = delete;
	blocking_queue& operator = (blocking_queue&&) = delete;
	blocking_queue& operator = (const blocking_queue&) = delete;

	~blocking_queue()
	{
		while (m_count--)
		{
			m_data[m_popIndex].~T();
			m_popIndex = ++m_popIndex % m_size;
		}
		operator delete(m_data);
	}

	void push(const T& item)
	{
		m_openSlots.wait();
	{
		std::lock_guard<std::mutex> lock(m_cs);
		try
		{
			new (m_data + m_pushIndex) T (item);
		}
		catch (...)
		{
			m_openSlots.post();
			throw;
		}
		m_pushIndex = ++m_pushIndex % m_size;
		++m_count;
	}
	m_fullSlots.post();
	}

	void pop(T& item)
	{
		m_fullSlots.wait();
	{
		std::lock_guard<std::mutex> lock(m_cs);
		try
		{
			item = m_data[m_popIndex];
		}
		catch (...)
		{
			m_fullSlots.post();
			throw;
		}
		m_data[m_popIndex].~T();
		m_popIndex = ++m_popIndex % m_size;
		--m_count;
	}
	m_openSlots.post();
	}

    bool empty()
    {
        std::lock_guard<std::mutex> lock(m_cs);
        return m_count == 0;
    }

    int n_(){

        return m_count;

    }

private:
	unsigned int m_size;
	unsigned int m_pushIndex;
	unsigned int m_popIndex;
    unsigned int m_count;
	T* m_data;

    boost::interprocess::interprocess_semaphore m_openSlots;
	boost::interprocess::interprocess_semaphore m_fullSlots;
    std::mutex m_cs;
};
