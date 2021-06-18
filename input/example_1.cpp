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
        return 3 % 2;
    }

    template<class T>
    int Kruh<T>::boo()
    {
        int x = 1;
        int y = 2;
        x += 1;
        return (x + y) + this->radius_;
    }
}