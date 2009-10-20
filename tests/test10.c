struct xyz { int x; int y; int z; };

int foo(struct xyz * x)
{
    return x->x + x->y + x->z;
}

int main(int argc, char * argv[])
{
    struct xyz x;

    printf("%d!\n", foo(&x));

    return 0;
}
