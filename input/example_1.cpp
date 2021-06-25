// #include <stddef.h>

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
        double area () const override
        {
            return 3.14;
        }

        int foo(int q);
        int boo();

    private:
        double radius_;
        int    weight_;
    };

    template<class T>
    int Kruh<T>::foo(int q)
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