#ifndef INT_ARRAY_H
#define INT_ARRAY_H

#include <cassert>
#include <initializer_list>

namespace Container
{
    class IntArray
    {
    private:// Private verables
        int* m_data;
        int m_length;


    public:// Public verables


    private:// Private Methods
        IntArray(const IntArray&) = delete;


    public:// Public Methods
        IntArray(void) : m_data{nullptr}, m_length{0} {}

        IntArray(int length);

        IntArray(std::initializer_list<int> list);

        IntArray& operator=(std::initializer_list<int> list);

        IntArray& operator=(IntArray& array);

        ~IntArray(void);

        void erase(void);

        int* begin(void) {return m_data;}

        int* end(void) {return (m_data + m_length);}

        int& operator[](int index);

        int getLength(void) { return m_length; }

        // Will remove every element from the array and then resize
        void reallocate(int length);

        // Will resize without delete the elements
        void resize(int length);

        void insertBefore(int value, int index);

        void remove(int index);

        void insertAtBeginning(int value){insertBefore(value, 0);}

        void insertAtEnd(int value){insertBefore(value, m_length);}
    };

}
#endif
