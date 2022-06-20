#include "IntArray.h"

namespace Container
{

    IntArray::IntArray(int length) : m_data{nullptr}, m_length{length}
    {
        assert(length >= 0 && "We can't have the length less than zero!");

        if(length > 0)
        {
            m_data = new int[(long unsigned int)length];
        }
    }

    IntArray::IntArray(std::initializer_list<int> list)
        : IntArray(static_cast<int>(list.size()))
    {
        int counter{};
        for(const auto& elem : list)
        {
            m_data[counter] = elem;
            ++counter;
        }
    }

    IntArray::~IntArray(void)
    {
        delete[] m_data;
    }

    IntArray& IntArray::operator=(std::initializer_list<int> list)
    {
        if(m_length <= static_cast<int>(list.size()))
        {
            resize(static_cast<int>(list.size()));
        }
        int counter{};
        for(auto elem : list)
        {
            m_data[counter] = elem;
            ++counter;
        }
        return *this;
    }

    IntArray& IntArray::operator=(IntArray& array)
    {
        if(this == &array)
        {
            return *this;
        }

        m_length = array.m_length;

        for(int counter{}; auto elem : array)
        {
            m_data[counter] = elem;
            ++counter;
        }
        return *this;
    }

    void IntArray::erase(void)
    {
        delete[] m_data;

        m_data = nullptr;
        m_length = 0;
    }

    int& IntArray::operator[](int index)
    {
        assert(index >= 0 || index < m_length);
        return m_data[index];
    }

    void IntArray::reallocate(int length)
    {
        erase();

        if(length <= 0)
            return;

        m_length = length;
        m_data = new int[(unsigned long)length];
    }

    void IntArray::resize(int length)
    {
        if(length == m_length)
        {
            return;
        }
        else if(length <= 0)
        {
            erase();
            return;
        }

        int* newData{new int[(unsigned long)length]};

        if(m_length > 0)
        {
            int elemToCopy{(m_length > length)? length : m_length};

            for(int counter{0}; counter < elemToCopy ; ++counter)
            {
                newData[counter] = m_data[counter];
            }

        }

        delete[] m_data;

        m_data = newData;

        m_length = length;
    }

    void IntArray::insertBefore(int value, int index)
    {
        assert(index >= 0 || index <= m_length);

        int* newData = new int[(unsigned long)(m_length+1)];

        for(int before{0}; before < index; ++before)
        {
            newData[before] = m_data[before];
        }

        newData[index] = value;

        for(int after{index}; after < m_length; ++after)
        {
            newData[after + 1] = m_data[after];
        }

        delete[] m_data;

        m_data = newData;
        ++m_length;
    }

    void IntArray::remove(int index)
    {
        assert(index >= 0 || index <= m_length);

        if(m_length == 1)
        {
            erase();
            return;
        }

        int* newData = new int[(unsigned long)(m_length -1)];

        for(int before{0}; before < index; ++before)
        {
            newData[before] = m_data[before];
        }


        for(int after{index + 1}; after< m_length; ++after)
        {
            newData[after - 1] = m_data[after];
        }

        delete[] m_data;

        m_data = newData;
        --m_length;
    }

}
