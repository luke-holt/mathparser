# Overview
Convert a math expression to it's equivalent in Reverse Polish Notation (RPN).

# Shunting Yard Algorithm
[wikipedia](https://en.wikipedia.org/wiki/Shunting_yard_algorithm#The_algorithm_in_detail)

# Example Usage
```
$ ./parsemath
usage: ./parsemath <expr>
supported operations: [ +, -, *, /, ** ]
$ ./parsemath "1 + 1"
tokens: 1 + 1
rpn: 1 1 +
result: 2
$ ./parsemath "(24 - 9 * 2) ** 2"
tokens: ( 24 - 9 * 2 ) ** 2
rpn: 24 9 2 * - 2 **
result: 36
$ ./parsemath "108 / (2 ** 3 + 4)"
tokens: 108 / ( 2 ** 3 + 4 )
rpn: 108 2 3 ** 4 + /
result: 9
```
