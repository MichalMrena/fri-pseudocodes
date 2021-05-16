namespace aud
{
    template<class T>
    class List
    {
    public:
        int foo();
        int boo();

    private:
        double member_;
    };

    template<class T>
    int List<T>::boo()
    {
        int x = 1;
        int y = 2;
        return 3 * (x + y);
    }
}