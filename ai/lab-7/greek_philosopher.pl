% Greek Philosopher Problem

% Step 1: Predicate logic representation
mortal(X) :- man(X).
man(X) :- greek(X).
greek(socrates).

% Step 2: Negation of goal
negated_goal(not(mortal(socrates))).

% Step 3: Implication removal
remove_implications(implies(man(X), mortal(X)), or(not(man(X)), mortal(X))).
remove_implications(implies(greek(X), man(X)), or(not(greek(X)), man(X))).

% Step 4: Negation movement
% Formulas are already in NNF.

% Step 5: Standardization
% Variables renamed to X1 and X2.

% Step 6: Skolemization
% No existential quantifiers present.

% Step 7: CNF clauses
% C1: [not(man(X1)), mortal(X1)], C2: [not(greek(X2)), man(X2)], C3: [greek(socrates)], C4: [not(mortal(socrates))]

% Step 8: Resolution implementation and execution
complementary(not(P), P).
complementary(P, not(P)).

resolve(C1, C2, Resolvent) :-
    select(L1, C1, R1),
    select(L2, C2, R2),
    complementary(L1, L2),
    append(R1, R2, Temp),
    sort(Temp, Resolvent).

prove :-
    C1 = [not(man(X1)), mortal(X1)],
    C2 = [not(greek(X2)), man(X2)],
    C3 = [greek(socrates)],
    C4 = [not(mortal(socrates))],

    write('Clause 1: '), writeln(C1),
    write('Clause 2: '), writeln(C2),
    write('Clause 3: '), writeln(C3),
    write('Negated goal: '), writeln(C4),
    nl,

    resolve(C4, C1, R1),
    write('Resolution 1: '), writeln(R1),

    resolve(R1, C2, R2),
    write('Resolution 2: '), writeln(R2),

    resolve(R2, C3, R3),
    write('Resolution 3: '), writeln(R3),

    write('Empty clause: '), writeln(R3),
    writeln('Contradiction found!'),
    writeln('Therefore, Socrates is mortal is proved.'),
    !.
