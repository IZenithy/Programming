#ifndef INT_ARRAY_H
#define INT_ARRAY_H

#include <cassert>
#include <initializer_list>

namespace Container
{
    template<typename T>
    class Vector
    {
    private:// Private verables
        T* m_data;
        int m_length;


    public:// Public verables


    private:// Private Methods
        Vector(const Vector&) = delete;


    public:// Public Methods
        Vector(void) : m_data{nullptr}, m_length{0} {}

        Vector(int length);

        Vector(std::initializer_list<T> list);

        Vector& operator=(std::initializer_list<T> list);

        Vector& operator=(Vector& vector);

        ~Vector(void);

        void erase(void);

        T* begin(void) {return m_data;}

        T* end(void) {return (m_data + m_length);}

        T& operator[](int index);

        int getLength(void) { return m_length; }

        // Will remove every element from the vector and then resize
        void reallocate(int length);

        // Will resize without delete the elements
        void resize(int length);

        void insertBefore(T value, int index);

        void remove(int index);

        void insertAtBeginning(T value){insertBefore(value, 0);}

        void insertAtEnd(T value){insertBefore(value, m_length);}
    };





    template<typename T>
    Vector<T>::Vector(int length) : m_data{nullptr}, m_length{length}
    {
        assert(length >= 0 && "We can't have the length less than zero!");

        if(length > 0)
        {
            m_data = new T[(long unsigned int)length];
        }
    }

    template<typename T>
    Vector<T>::Vector(std::initializer_list<T> list)
        : Vector(static_cast<int>(list.size()))
    {
        int counter{};
        for(const auto& elem : list)
        {
            m_data[counter] = elem;
            ++counter;
        }
    }

    template<typename T>
    Vector<T>::~Vector(void)
    {
        delete[] m_data;
    }

    template<typename T>
    Vector<T>& Vector<T>::operator=(std::initializer_list<T> list)
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

    template<typename T>
    Vector<T>& Vector<T>::operator=(Vector<T>& vector)
    {
        if(this == &vector)
        {
            return *this;
        }

        m_length = vector.m_length;

        for(int counter{}; auto elem : vector)
        {
            m_data[counter] = elem;
            ++counter;
        }
        return *this;
    }

    template<typename T>
    void Vector<T>::erase(void)
    {
        delete[] m_data;

        m_data = nullptr;
        m_length = 0;
    }

    template<typename T>
    T& Vector<T>::operator[](int index)
    {
        assert(index >= 0 || index < m_length);
        return m_data[index];
    }

    template<typename T>
    void Vector<T>::reallocate(int length)
    {
        erase();

        if(length <= 0)
            return;

        m_length = length;
        m_data = new T[(unsigned long)length];
    }

    template<typename T>
    void Vector<T>::resize(int length)
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

        T* newData{new T[(unsigned long)length]};

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

    template<typename T>
    void Vector<T>::insertBefore(T value, int index)
    {
        assert(index >= 0 || index <= m_length);

        T* newData = new T[(unsigned long)(m_length+1)];

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

    template<typename T>
    void Vector<T>::remove(int index)
    {
        assert(index >= 0 || index <= m_length);

        if(m_length == 1)
        {
            erase();
            return;
        }

        T* newData = new T[(unsigned long)(m_length -1)];

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
#endif
