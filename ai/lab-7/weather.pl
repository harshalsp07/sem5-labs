% Facts
warm.
raining.
sunny.
pleasant.

% Rules
enjoy :- sunny, warm.

strawberry_picking :- warm, pleasant.

not_strawberry_picking :- raining.

wet :- raining.



% ----------------------------
% Remove implications
% A -> B becomes ~A v B
% ----------------------------

remove_implications(
    implies(and(sunny, warm), enjoy),
    or(not(sunny), or(not(warm), enjoy))
).


remove_implications(
    implies(raining, wet),
    or(not(raining), wet)
).


% --------------------------------
%  Resolution Refutation
% --------------------------------

% Two literals are complementary if one is
% the negation of the other.

complementary(not(P), P).
complementary(P, not(P)).


% Resolution rule
resolve(C1, C2, Resolvent) :-
    select(L1, C1, R1),
    select(L2, C2, R2),
    complementary(L1, L2),
    append(R1, R2, Temp),
    sort(Temp, Resolvent).


% Resolution refutation for the goal: wet
prove :-
    % Clause 1: raining
    C1 = [raining],

    % Clause 2: not raining OR wet
    C2 = [not(raining), wet],

    % Negated goal: not wet
    C3 = [not(wet)],

    write('Clause 1: '), writeln(C1),
    write('Clause 2: '), writeln(C2),
    write('Negated goal: '), writeln(C3),
    nl,

    % Resolve raining with (not raining OR wet)
    resolve(C1, C2, R1),

    write('Resolution 1: '),
    writeln(R1),

    % Resolve wet with not wet
    resolve(R1, C3, R2),

    write('Resolution 2: '),
    writeln(R2),

    write('Empty clause: '),
    writeln(R2),

    writeln('Contradiction found!'),
    writeln('Therefore, wet is proved.').