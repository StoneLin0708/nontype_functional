#include "common_callables.h"

#include <memory>
#include <optional>

namespace
{

struct A
{
    int add(int a)
    {
        val += a;
        return val;
    }

    void set(int a) &noexcept { val = a; }

    int val = 0;
};

int neg(A const &a) noexcept
{
    return -a.val;
}

} // namespace

suite nttp_callable = []
{
    using namespace bdd;

    feature("type-erase a bound instance method") = []
    {
        given("an object without call operator") = []
        {
            A obj;

            when("binding a member function to a reference") = [=]() mutable
            {
                function fn = {nontype<&A::set>, std::ref(obj)};

                then("you can observe reference semantics") = [&]
                {
                    fn(11);
                    expect(obj.val == 11_i);
                };

                when("rebinding a different memfn with a pointer") = [=]
                {
                    auto copy = obj;
                    mut(fn) = {nontype<&A::add>, &copy};

                    then("you can observe reference semantics too") = [&]
                    {
                        fn(6);
                        expect(copy.val == 17_i);
                    };
                };

                when("rebinding the same memfn and an object") = [=]
                {
                    mut(fn) = {nontype<&A::add>, obj};

                    then("you can observe value semantics instead") = [&]
                    {
                        fn(6);
                        expect(obj.val == 11_i);
                    };
                };
            };

            when("binding a free function to a reference") = [=]() mutable
            {
                function fn{nontype<neg>, std::cref(obj)};

                then("you can observe reference semantics") = [&]
                {
                    expect(fn() == 0_i);

                    obj.val = -3;
                    expect(fn() == 3_i);
                };

                when("rebinding a pointer-to-data-member to a reference") = [=]
                {
                    auto copy = obj;
                    mut(fn) = {nontype<&A::val>, std::ref(copy)};

                    then("you can observe reference semantics too") = [&]
                    {
                        expect(fn() == -3_i);

                        copy.val = 15;
                        expect(fn() == 15_i);
                    };
                };

                when("rebinding a PMD to a pointer-like object") = [=]
                {
                    then("you may get reference semantics if copies of the "
                         "object share states") = [&]
                    {
                        auto ptr = std::make_shared<A>(6);
                        mut(fn) = {nontype<&A::val>, ptr};

                        expect(fn() == 6_i);

                        ptr->val = 17;
                        expect(fn() == 17_i);
                    };

                    then("you may get value semantics if copies of the object "
                         "are distinct") = [&]
                    {
                        auto opt = std::make_optional<A>(6);
                        mut(fn) = {nontype<&A::val>, opt};

                        expect(fn() == 6_i);

                        opt->val = 17;
                        expect(fn() == 6_i);
                    };
                };

                when("rebinding a stateless closure to a copy") = [=]() mutable
                {
                    obj = {};
                    fn = {nontype<[](A &obj) { return ++obj.val; }>, obj};

                    then("you can observe value semantics") = [&]
                    {
                        expect(fn() == 1_i);
                        expect(obj.val == 0_i);
                        expect(fn() == 2_i);
                        expect(obj.val == 0_i);
                    };
                };

                test("non-member does not dereference pointers") = [=]() mutable
                {
                    obj = {};
                    fn = {nontype<[](A *obj) { return ++obj->val; }>, &obj};

                    then("observe reference semantics as requested") = [&]
                    {
                        expect(fn() == 1_i);
                        expect(obj.val == 1_i);
                        expect(fn() == 2_i);
                        expect(obj.val == 2_i);
                    };
                };
            };
        };
    };

    feature("type-erase an unbound instance method") = []
    {
        given("a function pointer NTTP") = [obj = A{6}]
        {
            function fn = nontype<neg>;

            then("the signature is copied") = [&]
            {
                expect(fn({-3}) == 3_i);
                expect(fn(obj) == -6_i);
            };

            then("you can rebind a pointer-to-member") = [=]
            {
                mut(fn) = nontype<&A::val>;

                expect(fn({-3}) == -3_i);
                expect(fn(obj) == 6_i);
            };
        };
    };
};