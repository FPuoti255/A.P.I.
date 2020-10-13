# A.P.I.
Algotithms&amp;PrinciplesOfComputerScience

The project is aimed to monitor relationships among several entities (for instance, people)
that could change over time.

•Let's immagine, for instance, a social network in which:
  - new users could sign up;
  - already existing people may delete their account;
  - new relationships could be bound as well as a relationship between two people could be unbound;
  ..so on and so forth.

Mind that relationships are not necessarily simmetric.
For example, Alice follows Bruno, but the latter may not follow her back.
Therefore, each realtionship is directed.
The "report" command print on standard output, for each type of relationship, the person who has the biggest number of ingoing relationships.

In the "suites" folder there are some examples of inputs, and the related outputs, of the program.
If you want to try it.
  1) compile it with gcc
  2) choose the desired file among the ones in the "suites" folder and pass it as an argument on stdin.
