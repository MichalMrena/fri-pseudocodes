namespace aud
{
    class Dummy;

    template<class T>
    class Kruh
    {
    public:
        int foo(int q);
        int boo();

    private:
        double radius_;
        int    weight_;
    };

    struct Base
    {

    };

    class Dummy : public Base
    {
        int q = 10;
    };

    template<class T>
    int Kruh<T>::boo()
    {
        int x = 1;
        int y = 2;
        return 3 * (x + y);
    }
}