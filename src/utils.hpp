#include <iterator>
#include <memory>

namespace fri
{
    template<class Iterator, class Predicate>
    class FilterIterator
    {
    public:
        using difference_type   = typename Iterator::defference_type;
        using value_type        = typename Iterator::value_type;
        using pointer           = typename Iterator::pointer;
        using reference         = typename Iterator::reference;
        using iterator_category = std::input_iterator_tag;

    public:
        FilterIterator (Iterator, Iterator, Predicate);

        auto operator*  () const -> reference;
        auto operator-> () const -> pointer;
        auto operator++ ()       -> FilterIterator&;
        auto operator++ (int)    -> FilterIterator;
        auto operator== (FilterIterator const&) const -> bool;
        auto operator!= (FilterIterator const&) const -> bool;

    private:
        Iterator  it_;
        Iterator  end_;
        Predicate p_;
    };

    template<class Iterator, class Predicate>
    FilterIterator<Iterator, Predicate>::FilterIterator
        (Iterator it, Iterator end, Predicate p) :
        it_  (it),
        end_ (end),
        p_   (p)
    {
    }

    template<class Iterator, class Predicate>
    auto FilterIterator<Iterator, Predicate>::operator*
        () const -> reference
    {
        return *it_;
    }

    template<class Iterator, class Predicate>
    auto FilterIterator<Iterator, Predicate>::operator->
        () const -> pointer
    {
        return std::addressof(**this);
    }

    template<class Iterator, class Predicate>
    auto FilterIterator<Iterator, Predicate>::operator++
        () -> FilterIterator&
    {
        while (it_ != end_ and not p_(*this))
        {
            ++it_;
        }
        return *this;
    }

    template<class Iterator, class Predicate>
    auto FilterIterator<Iterator, Predicate>::operator++
        (int) -> FilterIterator
    {
        auto const ret = *this;
        ++(*this);
        return ret;
    }

    template<class Iterator, class Predicate>
    auto FilterIterator<Iterator, Predicate>::operator==
        (FilterIterator const& rhs) const -> bool
    {
        return it_ == rhs.it_;
    }

    template<class Iterator, class Predicate>
    auto FilterIterator<Iterator, Predicate>::operator!=
        (FilterIterator const& rhs) const -> bool
    {
        return not (*this == rhs);
    }
}