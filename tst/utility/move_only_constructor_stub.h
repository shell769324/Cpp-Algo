#pragma once
namespace algo {

class move_only_constructor_stub {
public:
    static int default_constructor_invocation_count;
    static int move_constructor_invocation_count;
    static int move_assignment_operator_invocation_count;
    static int destructor_invocation_count;
    static int constructor_invocation_count;
    static int counter;


    int id;

    move_only_constructor_stub();

    move_only_constructor_stub(int id);

    move_only_constructor_stub(move_only_constructor_stub&& other);

    move_only_constructor_stub& operator=(move_only_constructor_stub&& other);

    static void reset_constructor_destructor_counter() noexcept;

    ~move_only_constructor_stub() noexcept;
};

bool operator==(const move_only_constructor_stub& a, const move_only_constructor_stub& b);
std::strong_ordering operator<=>(const move_only_constructor_stub& a, const move_only_constructor_stub& b);
}
