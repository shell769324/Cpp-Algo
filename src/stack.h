#pragma once

#include "common.h"
#include "deque/deque.h"

namespace algo {

/**
 * @brief A generic stack
 * 
 * @tparam T the type of elements in the stack 
 */
template <typename T, typename Container = deque<T> >
requires std::same_as<T, typename Container::value_type>
class stack {
public:
    using container_type = Container;
    using value_type = Container::value_type;
    using size_type = Container::size_type;
    using reference = Container::reference;
    using const_reference = Container::const_reference;

private:
    Container container;

public:
    /**
     * @brief Default constructor
     * 
     * Construct a new empty stack
     */
    stack() { }

    /**
     * @brief Construct a stack with default constructed elements
     * 
     * Construct a new stack of given size. All elements are
     * default constructed
     * 
     * @param n the size of the stack
     */
    stack(size_type n) requires std::default_initializable<T> 
        : container(n) { }

    /**
     * @brief Construct a stack with copied of a given element
     * 
     * Construct a new stack of a given length. All elements are
     * copys of the specified element
     * 
     * @param n the length of the stack
     * @param value the element to copy
     */
    stack(size_type n, const_reference value) requires std::copy_constructible<T> 
        : container(n, value) { }

    /**
     * @brief Construct a stack by copying a container
     * 
     * @param container the underlying container used by the stack
     */
    explicit stack(const Container& container) : container(container) { }

    /**
     * @brief Construct a stack by moving a container
     * 
     * @param container the underlying container used by the stack
     */
    explicit stack(Container&& container) : container(std::move(container)) { }

    /**
     * @brief Copy constructor
     * 
     * Construct a copy of a given stack without modifying it
     * 
     * @param other the stack to copy from
     */
    stack(const stack& other) requires std::copy_constructible<T> 
        : container(other.container) { }

    /**
     * @brief Move constructor
     * 
     * Construct a copy of a given stack by using its resource. The given
     * stack will be left in a valid yet unspecified state
     * 
     * @param other 
     */
    stack(stack&& other) noexcept {
        container.swap(other.container);
    }

    /**
     * @brief Construct a new stack object from range
     * 
     * @tparam InputIt the type of the iterators that specify the range
     * @param first the beginning of the range
     * @param last the end of the range
     */
    template<class InputIt>
    stack(InputIt first, InputIt last) : container(first, last) { }

    /**
     * @brief Construct a new stack object from an initializer list
     * 
     * Example: stack<int> vec{1, 2, 3, 4, 5}
     * 
     * @param list the initializer list
     */
    stack(std::initializer_list<value_type> list) : container(list) {}

    /**
     * @brief Assignment operator
     * 
     * Copy all elements from another stack into this one without modifying it
     * 
     * @param other the stack to copy from
     * @return a reference to itself
     */
    stack& operator=(const stack& other) requires std::copy_constructible<T> {
        if (this == &other) {
            return *this;
        }
        stack tmp(other);
        swap(tmp);
        // tmp will be recycled because it goes out of scope
        return *this;
    }


    /**
     * @brief Move assignment operator
     * 
     * Move all elements from another stack into this one. The given
     * stack will be left in a valid yet unspecified state
     * 
     * @param other the stack to move from
     * @return a reference to itself
     */
    stack& operator=(stack&& other) noexcept {
        swap(other);
        return *this;
    }

    /**
     * @return a reference to the top element in the stack
     */
    reference top() const {
        return container.back();
    }

    /**
     * @return true if the stack is empty, false otherwise
     */
    bool empty() const noexcept {
        return container.empty();
    }

    /**
     * @return the number of elements in the stack
     */
    size_type size() const noexcept {
        return container.size();
    }

    /**
     * @brief create and push a copy of the given value to the top
     * of the stack
     * 
     * @param value the value to copy and append
     */
    void push(const_reference value) requires std::copy_constructible<T> {
        container.push_back(value);
    }

    /**
     * @brief move and add given value to the top of the stack
     * 
     * The value will be left in an unspecified yet valid state
     * 
     * @param value the value to move from
     */
    void push(value_type&& value) requires std::move_constructible<T> {
        container.push_back(std::move(value));
    }

    /**
     * @brief construct a new element from the arguments and append it to the
     * end of the container
     * 
     * @tparam Args an variadic type for any combination of arguments
     * @param args the argument used to construct the element
     * @return reference to the appended element
     */
    template<typename... Args>
    reference emplace(Args&&... args) {
        return container.emplace_back(std::forward<Args>(args)...);
    }

    /**
     * @brief Remove the last element in the stack and destroy it
     */
    void pop() {
        container.pop_back();
    }

    /**
     * @brief swap the content with another stack
     * 
     * @param other the stack to swap with
     */
    void swap(stack& other) noexcept {
        std::swap(container, other.container);
    }

    /**
     * @brief Check equality of two stacks
     * 
     * @param stack1 the first stack
     * @param stack2 the second stack
     * @return true if their contents are equal, false otherwise
     */
    friend bool operator==(const stack& stack1, const stack& stack2) requires equality_comparable<value_type> {
        return stack1.container == stack2.container;
    }

    /**
     * @brief Compare two stacks
     * 
     * @param stack1 the first stack
     * @param stack2 the second stack
     * @return a strong ordering comparison result
     */
    friend std::strong_ordering operator<=>(const stack& stack1, const stack& stack2) requires less_comparable<value_type> {
        return stack1.container <=> stack2.container;
    }
};
}
