#pragma once

template <typename T>
class lliste
{
public:
    lliste(T data)
    {
        m_data = data;
        m_prev = nullptr;
        m_next = nullptr;
    }

    lliste(T data, lliste* prev)
    {
        m_data = data;
        m_prev = prev;
        m_next = nullptr;
    }

    lliste* m_prev;
    lliste* m_next;
    T m_data;

    lliste* head()
    {
        auto curr = this;
        while (curr->m_prev != nullptr) curr = curr->m_prev;
        return curr;
    }

    lliste* tail()
    {
        auto curr = this;
        while (curr->m_next != nullptr) curr = curr->m_next;
        return curr;

    }

    void append(T data)
    {
        auto new_entry = new lliste(data, this);
        m_next = new_entry;
    }
};
