namespace aud
{
    template<class T>
    class Kruh
    {
    public:
        int foo();
        int boo();

    private:
        double radius_;
        int    weight_;
    };

    template<class T>
    int Kruh<T>::boo()
    {
        int x = 1;
        int y = 2;
        return 3 * (x + y);
    }
}