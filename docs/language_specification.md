# Nothing Beats a Jet2 Holiday language specification

## Source code encoding

Nothing Beats a Jet2 Holiday source code is text encoded in ASCII.

## Program resources

Nothing Beats a Jet2 Holiday programs are given the following resources:

- a working memory of 65,536 (2^16) 8-bit bytes,
- a pointer initialized to point to the first byte of working memory,
  referred to as the *data pointer*,
- an input file,
- an output file.

They can use these resources through commands represented in source code by
keywords.

## Comments

Source code can contain comments that will be ignored during lexical
analysis. Comments start with the semicolon character (`';'`) and stop at the
end of the line.

## Keywords

Programs are a sequence of commands represented in source code by the following
keywords:

| Keyword   | Meaning                                                                                                                             |
|-----------|-------------------------------------------------------------------------------------------------------------------------------------|
| `darling` | Increment the data pointer by 1. It is an execution error to do so when it points to the last byte of the working memory.           |
| `hold`    | Decrement the data pointer by 1. It is an execution error to do so when it points to the first byte of the working memory.          |
| `my`      | Increment the value pointed to by the data pointer by 1. If it was 255, then it becomes 0.                                          |
| `hand`    | Decrement the value pointed to by the data pointer by 1. If it was 0, then it becomes 255.                                          |
| `nothing` | Write the value pointed to by the data pointer as character to the output file.                                                     |
| `beats`   | Read a character from the input file into the value pointed to by the data pointer.                                                 |
| `a`       | If the value pointed to by the data pointer is zero, then continue execution of the program after the matching `jet2` keyword.      |
| `jet2`    | If the value pointed to by the data pointer is not zero, then continue execution of the program after the matching `a` keyword.     |
| `holiday` | Call the debugging event handler.                                                                                                   |

## Whitespace

Keywords are separated by whitespace, defined as a sequence of one or more
of the following characters:

- horizontal tab (`'\t'`),
- new line (`'\n'`),
- carriage return (`'\r'`),
- space (`' '`).
