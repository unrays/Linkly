I'll probably add something here, but know that I worked incredibly hard on this little bit of code; in fact, I had to start from scratch at least seven times. I finally found the solution to all my problems: recreating the `next` statement of the chain using `using`, which allows me to make my links dynamic while maintaining 100% compile time. ;) I'm very proud of this result! Maybe I'll work on embellishing it as a library...?! Who knows!

*Also, this is my first project of 2026!*

I've learned so much since I started meta-programming just a month and three weeks ago! I'm really starting to see significant progress and I'm also beginning to tackle challenges that I'm increasingly passionate about! I can't wait to see what the future holds ;)

*This is definitely not finished yet, I still have many more ideas to implement.*

---

#### Output
```console
*********** END ***********
class std::tuple<double,int,double>
50
25
45.3
------
class std::tuple<int>
3
------
class std::tuple<double,char const * __ptr64>
25.6
d
------
***************************
```

#### Main
```cpp
int main() {
  using Pipeline =
     FunctionOperator<
       SubscriptOperator<
         FunctionOperator<>
  >>;
  
  Pipeline pipeline({});
  
  pipeline(50, 25, 45.3)[3](25.6, "d");
}
```

#### Code
```cpp
// Copyright (c) January 2026 Félix-Olivier Dumas. All rights reserved.
// Licensed under the terms described in the LICENSE file

#define OFF 0
#define ON 1

#define ENABLE_ALIAS OFF

struct End {
    template<typename FinalState>
    End(FinalState& s) {
        // SEUL ELEMENT RUNTIME DU SYSTÈME COMPLET ;)

        std::cout << "*********** END ***********\n";

        std::apply([&](auto&&... op_tuples) {

            ((std::apply([&](auto&&... args) {
                std::cout << typeid(op_tuples).name() << "\n";
                ((std::cout << args << "\n"), ...);
                std::cout << "------\n";
                }, op_tuples)), ...);
            }, s);

        std::cout << "***************************\n\n";
    }

    void end() {
        std::cout << "Operator chain ended!\n";
    }

    void get() {

    }
};

template<typename T>
struct is_terminal : std::false_type {};

template<>
struct is_terminal<End> : std::true_type {};

/********************************************/

struct MetaOperator {}; //metaoperator?

/********************************************/

struct StatelessOperator : MetaOperator {};

/********************************************/

template<typename CurrentState>
struct StatefulOperator : MetaOperator {
public:
    StatefulOperator() = default;
    StatefulOperator(CurrentState s) : state_(std::move(s)) {}

protected:
    CurrentState state_;
};

/********************************************/

template<typename Operator>
struct OperatorTraits : MetaOperator {};

template<
    template<std::size_t, typename, typename> class Operator,
    std::size_t Arity,
    typename Next,
    typename State
>
struct OperatorTraits<
    Operator<Arity, Next, State>
> : MetaOperator
{
    template<std::size_t T1, typename T2, typename T3>
    using template_type = Operator<T1, T2, T3>;

    static constexpr std::size_t arity = Arity;
    using next_type = Next;
};

/********************************************/

template<
    std::size_t Arity, // = 0; variadic supporte 0
    typename Next,
    typename CurrentState
>
struct FunctionOperator_:
    StatefulOperator<CurrentState>,
    OperatorTraits<FunctionOperator_<Arity, Next, CurrentState>>
{
    template<
        typename... Args,
        std::size_t Args_arity = sizeof...(Args),

        typename = std::enable_if_t<
            Arity == 0 ||
            Args_arity == Arity
        >
    >
    auto operator()(Args&&... args) {
        auto concat_state_args = std::tuple_cat(
            this->state_,
            std::make_tuple(
                std::make_tuple(std::forward<Args>(args)...)
            )
        );

        if constexpr (is_terminal<Next>::value)
            return End{ concat_state_args };
        else
            return Next::template template_type<
                Next::arity,
                typename Next::next_type,
                decltype(concat_state_args)
            > (std::move(concat_state_args));
    }
};

/********************************************/

template<
    std::size_t Arity,
    typename Next,
    typename CurrentState
>
struct SubscriptOperator_ :
    StatefulOperator<CurrentState>,
    OperatorTraits<SubscriptOperator_<Arity, Next, CurrentState>>
{
    template<
        typename ...Args,
        std::size_t Args_arity = sizeof...(Args),

        typename = std::enable_if_t<
        Arity == 0 ||
        Args_arity == Arity
        >
    >
    auto operator[](Args&&... arg) {
        auto concat_state_args = std::tuple_cat(
            this->state_,
            std::make_tuple(
                std::make_tuple(std::move(arg))
            )
        );

        if constexpr (is_terminal<Next>::value)
            return End{ concat_state_args };
        else
            return Next::template template_type<
                Next::arity,
                typename Next::next_type,
                decltype(concat_state_args)
            > (std::move(concat_state_args));
    }

    template<typename Arg> // pre-C++23
    auto operator[](Arg arg) {
        auto concat_state_args = std::tuple_cat(
            this->state_,
            std::make_tuple(
                std::make_tuple(std::move(arg))
            )
        );

        if constexpr (is_terminal<Next>::value)
            return End{ concat_state_args };
        else
            return Next::template template_type<
                Next::arity,
                typename Next::next_type,
                decltype(concat_state_args)
            > (std::move(concat_state_args));
    }
};


/********************************************/
 // MACRO MACRO MACRO MACRO MACRO MACRO MACRO!!!!

static constexpr std::size_t DEFAULT_ARITY_VALUE = 0;
using DEFAULT_NEXT_TYPE = End;
using DEFAULT_STATE_TYPE = std::tuple<>;

/********************************************/

template<
    std::size_t Arity = DEFAULT_ARITY_VALUE,
    typename Next = DEFAULT_NEXT_TYPE,
    typename State = DEFAULT_STATE_TYPE
>
struct FunctionOperator_n : FunctionOperator_<Arity, Next, State> {};

template<
    typename Next = DEFAULT_NEXT_TYPE,
    typename State = DEFAULT_STATE_TYPE
>
struct FunctionOperator : FunctionOperator_<DEFAULT_ARITY_VALUE, Next, State> {};

/********************************************/

template<
    std::size_t Arity = DEFAULT_ARITY_VALUE,
    typename Next = DEFAULT_NEXT_TYPE,
    typename State = DEFAULT_STATE_TYPE
>
struct SubscriptOperator_n : SubscriptOperator_<Arity, Next, State> {};

template<
    typename Next = DEFAULT_NEXT_TYPE,
    typename State = DEFAULT_STATE_TYPE
>
struct SubscriptOperator : SubscriptOperator_<DEFAULT_ARITY_VALUE, Next, State> {};

/********************************************/

#if ENABLE_ALIAS
    using DEFAULT_NEXT_TYPE = End;
    using DEFAULT_STATE_TYPE = std::tuple<>;

#define GENERATE_ALIAS(alias_name, backend_name) \
    template<typename N=DEFAULT_NEXT_TYPE, typename S=DEFAULT_STATE_TYPE> \
    using alias_name = backend_name<N,S>;

    GENERATE_ALIAS(FunctionOperator, FunctionOperator_);
    GENERATE_ALIAS(SubscriptOperator, SubscriptOperator_);
#endif
```
