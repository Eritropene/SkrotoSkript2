# SkrotoSkript (2.0)

Shitty programming language, now upgraded. What i just needed. Here's the syntax:

## Variables

Variables are all the lowercase letters **[a-z]**. And yes, there are only 26 variable available. Good luck.

### Primitives

There are 4 primitive types: **Integer**, **Floating Point**, **Boolean** and **String**.

### Values

You can initialize a variable using **"..."**. Inside the **""** you type the value you need, like a number ("23", "4.3", ...) or a literal ("hello", ...). You can **ONLY** use it for initialization, jumping, or printing to console.

## Operations

Operations are all the uppercase letters **[A-Z]**. Yeah there are only 26 operation available. Compact indeed.

All operations are usually like this: `Pxy`, where P is the operation, x and y are variables.

The operations are the following:

|Letter|Operation|Syntax|
|------|---------|------|
|A| Add (+) | Axy, AXY
|B| Boolean declaration | Bx"0", Bxy, BxY
|C| Function call | Ca (returns a value)
|D| Divide (/) | Dxy, DXY
|E| Equals (==) | Exy, EXY
|F| Float declaration | Fx"3.4", Fxy, FxY
|G| Greater than (>)
|H| Modulus (%)
|I| Int declaration
|J| Jump (with index or with variable!)
|K| Logic Not (!)
|L| Less than (<)
|M| Multiply (*)
|N| Input
|O| Output (print)
|P| Push to stack
|Q| Define jump label
|R| Return
|S| String declaration
|T| Logic And (&&)
|U| ...nothing?
|V| Logic Or (\|\|)
|W| Import function
|X| Conditional Jump (with index or with variable!)
|Y| Pop from stack
|Z| Subtract (-)

### Jumps
Hell yeah there is no if-statement, only conditional jumps. A jump can be used in two ways:

`...Qx...Jx` <- Here the jump teleports you to where Qx is declared, in whichever part of the code. 

`...J"123"...` <- Here the jump sets the instruction pointer to the number written. **WARNING!** Only real man can use this.

The instruction `Q` sets a jump label for that corresponding letter. It does not replace the variable, so they can be used simultaneously.

### Function call

First you have to import the function (which is a skr file) and then push the parameters and then call. Clear.

... ok so basically a _skrotofunction_ is just another skrotoprocess running nested inside the main program. Each skr file is both a program and a function. To use it, you have to import it to your program using `W` command.

**DON'T WORRY**, even if you assign a letter to a function, you can still use that letter to define a variable! Variables and functions here are separate concepts and can be used simultaneously with the same letter (I care about your sanity).

Recap, How to call a function:

1. Import the function you want to use (Ex: `Wf"factorial"`, where _factorial.skr_ is the program i want to import).
2. To call the function, use `C`.
3. To use it with parameters, first load them into the stack, with `P`.
4. To read the parameter from the stack, pop them out with `Y`.

YOU HAVE A STACK, AND YES YOU CAN USE IT. Be grateful.