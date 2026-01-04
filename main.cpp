// Copyright (c) January 2026 Félix-Olivier Dumas. All rights reserved.
// Licensed under the terms described in the LICENSE file

#include <iostream>

namespace v4 {
    #define OFF 0
    #define ON 1

    #define ENABLE_ALIAS ON

    struct End {
        template<typename FinalState>
        End(FinalState& s) {
            // SEULE ELEMENT RUNTIME DU SYSTÈME COMPLET ;)

            std::cout << "*********** END ***********\n";
            //std::cout << typeid(s).name() << "\n";

            /*std::apply([&](auto&&... args) {
                ((std::cout << args << "\n"), ...);
            }, s);*/

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

    struct StatelessOperator;

    template<typename CurrentState>
    struct StatefulOperator {
    public:
        StatefulOperator() = default;
        StatefulOperator(CurrentState s) : state_(std::move(s)) {}

    protected:
        CurrentState state_;
    };

    /********************************************/

    template<typename Operator>
    struct MetaOperator;

    template<
        template<typename, typename> class Operator,
        typename Next,
        typename State
    >
    struct MetaOperator<Operator<Next, State>> {
        template<typename T1, typename T2>
        using template_type = Operator<T1, T2>;
        using next_type = Next;

        //fonction inspect() qui return l'accès aux methodes?
    };

    /********************************************/

    template<
        typename Next,
        typename CurrentState
    >
    struct FunctionOperator_ :
        StatefulOperator<CurrentState>,
        MetaOperator<FunctionOperator_<Next, CurrentState>>
    {
        template<typename... Args>
        auto operator()(Args&&... args) {
            auto t_ = std::make_tuple(std::forward<Args>(args)...);

            auto concat_state_args = std::tuple_cat(
                this->state_,
                std::make_tuple(t_)
            );

            //faire un wrapper autours de tuple avec string name/id???

            if constexpr (is_terminal<Next>::value)
                return End{ concat_state_args };
            else
                return Next::template template_type<
                typename Next::next_type,
                decltype(concat_state_args)
                >(std::move(concat_state_args));
        }
    };

    //JE POURRAIS FAIRE UN HOOK onOperated(args) qui permet specialisation
    //ET ON CACHE l'implémentation de base avec le concat_state_args dans un crtp
    //genre FunctionOperatorBASE

    //en gros tu fais un type wrapper et tu le nomme whatever
    // pour le ecs par exemple, on ferait std::get<type en question>

    /********************************************/

    template<
        typename Next,
        typename CurrentState
    >
    struct SubscriptOperator_:
        StatefulOperator<CurrentState>,
        MetaOperator<SubscriptOperator_<Next, CurrentState>>
    {
        template<typename Arg>
        auto operator[](Arg arg) {
            auto t_ = std::make_tuple(std::move(arg));

            auto concat_state_args = std::tuple_cat(
                this->state_,
                std::make_tuple(t_)
            );

            if constexpr (is_terminal<Next>::value) {
                std::cout << "sub op!\n";
                return End{ concat_state_args };
                // return concat_state_args;
            }

            else {
                //return Next(std::move(concat_state_args));
                std::cout << "sub op!\n";

                return Next::template template_type<
                    typename Next::next_type,
                    decltype(concat_state_args)
                > (std::move(concat_state_args));
            }

            //faudrait recreer une value ou some shit
        }
    };

    #if ENABLE_ALIAS
        using DEFAULT_NEXT_TYPE = End;
        using DEFAULT_STATE_TYPE = std::tuple<>;
        //faire same pour filter, juste mettre valeur par def

        #define GENERATE_ALIAS(alias_name, backend_name) \
            template<typename N=DEFAULT_NEXT_TYPE, typename S=DEFAULT_STATE_TYPE> \
            using alias_name = backend_name<N,S>;

        GENERATE_ALIAS(FunctionOperator, FunctionOperator_);
        GENERATE_ALIAS(SubscriptOperator, SubscriptOperator_);
    #endif
}

int main() {
  {
      using namespace v4;
  
      using Pipeline =
          FunctionOperator<
              SubscriptOperator<
                  FunctionOperator<>
              >
          >;
  
      //faire un system de tags avec partial specialization qui 
      //permet par exemple de dire que je veux qu'ils soient tous
      //le même type ou n elements
  
  
      Pipeline pipeline({});
  
      pipeline(50.4, 25, 45.3)[3](25.6, "d");
  
      //ca return le tuple a la fin ou faire un end qui .get<type ou identifiant>
  
      //FAIRE UN FACADE QUI TRANSFORME APPEL EN OP1 | OP2 | OP3
  }
}
