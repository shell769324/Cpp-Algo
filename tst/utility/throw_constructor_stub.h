#pragma once
namespace algo {

class throw_constructor_stub {
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

    throw_constructor_stub();

    throw_constructor_stub(int id);

    throw_constructor_stub(const throw_constructor_stub& other);

    throw_constructor_stub(throw_constructor_stub&& other);

    throw_constructor_stub& operator=(const throw_constructor_stub& other);

    throw_constructor_stub& operator=(throw_constructor_stub&& other);

    static void reset_constructor_destructor_counter() noexcept;

    ~throw_constructor_stub() noexcept;
};

bool operator==(const throw_constructor_stub& a, const throw_constructor_stub& b);
}
