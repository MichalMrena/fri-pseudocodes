#include <stddef.h>

namespace mm
{
    class Object
    {
        int uid;
    };

    class ISerializable
    {
    };

    class Tvar
    {
        virtual ~Tvar () = default;
        virtual double area () const = 0;
    };

    template<class T>
    class Kruh : public Tvar, public Object, public ISerializable
    {
    public:
        Kruh(int, int);

        double area () const override
        {
            return 3.14;
        }

        int foo(int q, int a);
        int boo();
        bool test()
        {
            return 1 || 0 && false or true;
        }

    private:
        double radius_;
        int    weight_;
    };

    template<class T>
    int Kruh<T>::foo(int q, int a)
    {
        if (q > 10)
        {
            int x = q + 10;
            while (x < q + 20)
            {
                q += 2;
            }
            return x % 2;
        }
        else
            return q % 3;
    }

    template<class T>
    int Kruh<T>::boo()
    {
        Kruh* k = new Kruh<T> (1, 2);
        int x = 1;
        int y = 2;
        do
        {
            x += 1;
        } while (x < 100);
        for (int i = 0; i < 10; ++i)
        {
            y += i;
        }
        return (x + y) + this->radius_;
    }
}