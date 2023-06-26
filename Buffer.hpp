class Buffer
{
public:
    Buffer(int x) {};

    static unsigned constexpr BUFER_SIZE = 512;
    char data[BUFER_SIZE];
};