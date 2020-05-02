#ifndef BUFLIST_HPP
#define BUFLIST_HPP

#include <cstdint>
#include <cstring>
#include <stddef.h>

namespace Hal
{

template<const size_t BUF_SIZE, const size_t ITEM_NUM>
class BufList
{
public:
    BufList()
        :
        m_size(0),
        m_capacity(ITEM_NUM),
        m_front(nullptr),
        m_back(nullptr)
    {
        // Left blank on purpose
    }

    ~BufList()
    {
        // Left blank on purpose
    }

    size_t item_size(void)
    {
        return BUF_SIZE;
    }

    size_t capacity(void)
    {
        return m_capacity;
    }

    size_t size(void)
    {
        return m_size;
    }

    std::uint8_t * push_front(void)
    {
        // If full, return nullptr
        if(m_size == ITEM_NUM)
        {
            return nullptr;
        }

        buf_item * tmp = pick_free();
        // If list is empty
        if(m_size == 0)
        {
            m_back = m_front = tmp;
            tmp->next = tmp->prev = nullptr;
        }
        else
        {
            tmp->prev = m_front;
            m_front->next = tmp;
            m_front = tmp;
            m_front->next = nullptr;
        }

        // Increment used items
        m_size++;
        return tmp->buffer;
    }

    std::uint8_t * front(void)
    {
        return m_front->buffer;
    }

    void pop_front(void)
    {
        if(m_size == 0)
        {
            return;
        }


        std::memset(m_front->buffer, 0, BUF_SIZE);
        if(m_size == 1)
        {
            m_front->used = false;
            m_front->next = nullptr;
            m_front->prev = nullptr;
            m_front = m_back = nullptr;
        }
        else
        {
            m_front->used = false;
            buf_item * tmp = m_front;
            m_front = m_front->prev;
            m_front->next = nullptr;
            tmp->next = tmp->prev = nullptr;
        }
        m_size--;
    }



    std::uint8_t * push_back(void)
    {
        // If full, return nullptr
        if(m_size == ITEM_NUM)
        {
            return nullptr;
        }

        buf_item * tmp = pick_free();
        // If list is empty
        if(m_size == 0)
        {
            m_back = m_front = tmp;
            m_back->next = m_back->prev = nullptr;

        }
        else
        {
            tmp->next = m_back;
            m_back->prev = tmp;
            m_back = tmp;
            m_back->prev = nullptr;

        }


        // Increment used items
        m_size++;
        return tmp->buffer;
    }

    std::uint8_t * back(void)
    {
        return m_back->buffer;
    }

    void pop_back(void)
    {
        if(m_size == 0)
        {
            return;
        }

        std::memset(m_back->buffer, 0, BUF_SIZE);
        if(m_size == 1)
        {
            m_back->used = false;
            m_back->next = m_back->prev = nullptr;
            m_back = m_front = nullptr;
        }
        else
        {
            m_back->used = false;
            buf_item * tmp = m_back;
            m_back = m_back->next;
            m_back->prev = nullptr;
            tmp->next = tmp->prev = nullptr;
        }
        m_size--;
    }

private:

    class buf_item
    {
        friend BufList;

    public:
        buf_item(void) : next(nullptr), prev(nullptr), used(false) {}
        std::uint8_t buffer[BUF_SIZE];
    private:
        buf_item * next;
        buf_item * prev;
        bool used;
    };

    size_t m_size;
    size_t m_capacity;
    buf_item m_items[ITEM_NUM];
    buf_item * m_front;
    buf_item * m_back;

    buf_item * pick_free(void)
    {
        buf_item * tmp = nullptr;
        if(m_size < ITEM_NUM)
        {
            for(uint32_t i = 0 ; i < ITEM_NUM ; i++)
            {
                if(!m_items[i].used)
                {
                    m_items[i].used = true;
                    tmp = &m_items[i];
                    break;
                }
            }
        }
        return tmp;
    }

}; // BufList

} // namespace Hal
#endif // BUFLIST_HPP
