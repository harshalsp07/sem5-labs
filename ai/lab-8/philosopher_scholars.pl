thinker(X) :- philosopher(X).
scholar(X) :- thinker(X).
knowledgeable(X) :- scholar(X).
philosopher(aristotle).

negated_goal(not(knowledgeable(aristotle))).

remove_implications(implies(philosopher(X), thinker(X)), or(not(philosopher(X)), thinker(X))).
remove_implications(implies(thinker(X), scholar(X)), or(not(thinker(X)), scholar(X))).
remove_implications(implies(scholar(X), knowledgeable(X)), or(not(scholar(X)), knowledgeable(X))).

negation_movement(or(not(philosopher(X)), thinker(X)), or(not(philosopher(X)), thinker(X))).
negation_movement(or(not(thinker(X)), scholar(X)), or(not(thinker(X)), scholar(X))).
negation_movement(or(not(scholar(X)), knowledgeable(X)), or(not(scholar(X)), knowledgeable(X))).

standardization(clause1, [not(philosopher(X1)), thinker(X1)]).
standardization(clause2, [not(thinker(X2)), scholar(X2)]).
standardization(clause3, [not(scholar(X3)), knowledgeable(X3)]).

skolemization(none).

cnf_clause(c1, [not(philosopher(X1)), thinker(X1)]).
cnf_clause(c2, [not(thinker(X2)), scholar(X2)]).
cnf_clause(c3, [not(scholar(X3)), knowledgeable(X3)]).
cnf_clause(c4, [philosopher(aristotle)]).
cnf_clause(c5, [not(knowledgeable(aristotle))]).

complementary(not(P), P).
complementary(P, not(P)).

resolve(C1, C2, Resolvent) :-
    select(L1, C1, R1),
    select(L2, C2, R2),
    complementary(L1, L2),
    append(R1, R2, Temp),
    sort(Temp, Resolvent).

prove :-
    C1 = [not(philosopher(X1)), thinker(X1)],
    C2 = [not(thinker(X2)), scholar(X2)],
    C3 = [not(scholar(X3)), knowledgeable(X3)],
    C4 = [philosopher(aristotle)],
    C5 = [not(knowledgeable(aristotle))],

    write('Clause 1: '), writeln(C1),
    write('Clause 2: '), writeln(C2),
    write('Clause 3: '), writeln(C3),
    write('Clause 4: '), writeln(C4),
    write('Negated goal: '), writeln(C5),
    nl,

    resolve(C5, C3, R1),
    write('Resolution 1: '), writeln(R1),

    resolve(R1, C2, R2),
    write('Resolution 2: '), writeln(R2),

    resolve(R2, C1, R3),
    write('Resolution 3: '), writeln(R3),

    resolve(R3, C4, R4),
    write('Resolution 4: '), writeln(R4),

    write('Empty clause: '), writeln(R4),
    writeln('Contradiction found!'),
    writeln('Therefore, Aristotle is knowledgeable is proved.'),
    !.
