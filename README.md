# BrainBang
BrainBang is a human-readable form of Brainf*ck. It keeps the same concept but has a dedicated syntax and is easier to read and write.

## Syntax
- BrainBang has a signficant whitespace. Code in loops must be indented similar to python
- All lines must end with a semicolon

## Commands
- ent (value): Enters a value into the cell
- inc (value): Increases the value of the cell by one or the specified number
- dec (value): Decreases the value of the cell by one or the specified number
- cellout: Outputs the value of the cell
- loop: Loops the code just like in Brainf*ck
- ent in: Reads a value from the user and enters it into the cell

## Example
```
ent 0;
inc 1;
loop:
    cellout;
    >;
```
