# UwU
UwU is an esoteric statically typed programming language that is very simple and easy to use.


# Installation
## Prerequisites
- GNU Make 4.0 or above


## Build
To clone the repository:
```
git clone https://github.com/PhoenixHO-dev/UwU.git
```
To build:
```
make
```
To run:
```
uwu
```
to activate a REPL session,
or
```
uwu <path> [-p | -e]
```
to execute a `.uwu` file.
- `<path>` is the path of the `.uwu` file.
- The optional flag `-p` can be used to print code instructions for debugging, while `-e` can be used to trace program execution.




# Language Features/Syntax
## Identifiers
**UwU** doesn't support functions or classes (yet), so Identifiers are just equivalent to variable names.
Identifiers can be alphanumerical (and can contain '\_') but they can only start with a letter or an underscore **'\_'**.
It should be noted that **UwU** is case sensitive, meaning that 'Var' and 'var' are two different identifiers.
## Variables
Variable declaration syntax:
```
uwu variable_name
```
To assign a value to the variable:
```
variable_name := value
```
Variables can also be initialized in their declaration like so:
```
uwu variable_name := value
```


## Data Types
**UwU** is a statically typed programming language and you don't need to specify the data type when declaring a variable. It supports the following data types:
- Numbers (integers, decimal numbers, ...)
- Characters
- Strings
- Boolean

To declare a number variable:
```
uwu number := value
```
`value` can be any number (integer or decimal number).

To declare a string variable:
```
uwu string := "This is a string"
```

To declare a character variable:
```
uwu character := `c`
```

To declare a boolean variable:
```
uwu boolean := value
```
Here `value` can either be `twue` (equivalent to `true` in C++) or `fawse` (equivalent to `false` in C++).

Variables of a specific type can also be assigned values of other types, meaning that the new value overwrites the old one including the data type:
```
uwu var := 1     {: This is a number variable :}
var := "string"  {: This operation is still valid but 'var' is no longer of type number :}
```


## Comments
**UwU** currently only supports multi-line comments.
A comment in **UwU** starts with `{:` and ends with `:}` and anything inside will be entirely ingnored.
Example:
```
{: This is a comment and it will be ignored during compilation :}
```


## Binary/Unary operators
- Addition `+`
- Subtraction `-`
- Multiplication `*`
- Division `/`
- Concatenation `+` (can be used with both characters and strings)
- Not `!`
- Negation `-`
- Comparison `=`, `!=`, `<`, `<=`, `>`, `>=` (Strings and Chars can be compared using the operators `=` and `!=` too)
- Logical Operators `awnd`, `ow`


## Variable Scope
**UwU** supports both **local** scope and **global** scope. A local scope/block is a set of statements put within a block.
A block is a region enclosed by `[:` and `:]`.
Similar to other programming languages like C or C++, variables declared within a block can only be accessed within that block, and are called local variables,
in contrast with global variables that can be accessed anywhere in the program.


## If statements
The syntax for an if-statement in **UwU** is:
```
?w? condition
[:
    {: code :}
:]
```
**UwU** also supports `if..else` statements:
```
?w? condition
[:
    {: executes if 'condition' is true :}
:]
ewe
[:
    {: executes otherwise :}
:]
```
Note that any non-zero value corresponds to `twue` while `0` corresponds to `fawse`.


## Loops
Loops in **UwU** are different than `while` loops in C/C++ in that the code within the following block executes until a certain condition is met.  
The syntax for `untiw` loops is:
```
untiw condition
[:
    {: executes until 'contidion' is true :}
:]
```


## Printing expressions
The syntax for printing an expression is:
```
ouo expression >>
```
`expression` can be any valid expression in **UwU**.  
To print multiple expressions:
```
ouo expression1, expression2, ... >>
```
To print a new line:
```
ouo ~n >>
```
To print a tab:
```
ouo ~t >>
```


## Reading input
Reading input is different for each data type. The syntax is:
```
iwi type variable_name <<
```
`type` can be `-s`, `-d`, or `-c`.  
`-s` if for strings.  
`-d` is for numbers.  
`-c` is for characters.  
Example:
```
iwi-s string_var << {: Reads a string from input to 'string_var' :}
```


# Supported operations
- Using binary operator ```+``` on strings/characters:  
    `` "string" + `a` `` evaluates to `"stringa"`  
    `` `a` + "string" `` evaluates to `"astring"`  
    `` `a` + `b` ``      evaluates to `"ab"`  
- Using equality operators on non-numbers always evaluates to `fawse`.
- Assigning a value of a specific type to a variable of a different data type, that includes reading from input.
- Using non-boolean expressions as if-statement or loop condition.



# UwU Program Examples
**UwU** "Hello World!" program:
```
ouo "Hewwo Wowwd!" >>
```

**UwU** program to check if the entered number is positive or negative:
```
uwu number

ouo "Enter a number: " >>
iwi-d number <<

?w? number <= 0 [:
	?w? number = 0 [:
		ouo ~n + "You entered a null number." >>
	:] ewe [:
		ouo ~n + "You entered a negative number." >>
	:]
:] ewe [:
	ouo ~n, "You entered a positive number." >>
:]
```

**UwU** program to print Fibonacci Sequence:
```
uwu t1 := 0
uwu t2 := 1
uwu nextTerm := t1 + t2

uwu n

ouo "Enter the number of terms: " >>
iwi-d n <<

ouo "Fibonacci Series: " + ~n >>
ouo t1, ", " >>
ouo t2, ", " >>

uwu i := 3
untiw i > n [:
	ouo nextTerm, ", " >>
	t1 := t2
	t2 := nextTerm
	nextTerm := t1 + t2
	i := i + 1
:]
```









