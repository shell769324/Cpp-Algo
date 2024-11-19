#pragma once
namespace algo {

class copy_only_constructor_stub {
public:
    static int default_constructor_invocation_count;
    static int copy_constructor_invocation_count;
    static int assignment_operator_invocation_count;
    static int destructor_invocation_count;
    static int constructor_invocation_count;
    static int counter;


    int id;

    copy_only_constructor_stub();

    copy_only_constructor_stub(int id);

    copy_only_constructor_stub(const copy_only_constructor_stub& other);

    copy_only_constructor_stub(copy_only_constructor_stub&& other) = delete;

    copy_only_constructor_stub& operator=(const copy_only_constructor_stub& other);

    copy_only_constructor_stub& operator=(copy_only_constructor_stub&& other) = delete;

    static void reset_constructor_destructor_counter() noexcept;

    ~copy_only_constructor_stub() noexcept;
};

bool operator==(const copy_only_constructor_stub& a, const copy_only_constructor_stub& b);
}
