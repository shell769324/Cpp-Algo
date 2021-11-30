namespace algo {

class constructor_stub {
public:
    static int default_constructor_invocation_count;
    static int copy_constructor_invocation_count;
    static int move_constructor_invocation_count;
    static int assignment_operator_invocation_count;
    static int move_assignment_operator_invocation_count;
    static int destructor_invocation_count;
    static int constructor_invocation_count;
    static int counter;


    int id;

    constructor_stub();

    constructor_stub(int id);

    constructor_stub(const constructor_stub& other);

    constructor_stub(constructor_stub&& other) noexcept;

    constructor_stub& operator=(const constructor_stub& other);

    constructor_stub& operator=(constructor_stub&& other) noexcept;

    static void reset_constructor_destructor_counter() noexcept;

    ~constructor_stub() noexcept;
};

bool operator==(const constructor_stub& a, const constructor_stub& b);
}