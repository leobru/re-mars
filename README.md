# MARS-6 archive system

MARS-6 was an implementation of an external sorted key-value container.
The API mechanism was in form of a micro-program.

Pascal compilers for the BESM-6 included a few MARS-6 operations as part ot the standard library, encoded as micro-programs.
They are:

| Operation | Micro-program | Comment |
| --- | --- | --- |
| GETD | 11-14-22 | Get the value by key, error out if the key does not exist |
| PUTD | 11-15-21-26 | Add a key-value pair, error out if the key elready exists |
| DELD | 11-14-23-27 | Delete a pair by key, error out if the key does not exist |
| MODD | 11-15-00-21-26-40-20 | Modify the value if a pair with the key already exists, add a pair otherwise |
| OPEND | 31-11-14-12-25 | Open an existing container by name, error out if it does not exist in the root catalog |
| NEWD | 31-11-15-21-26-11-14-12-10-21 | Create a container given a name and LUN/starting block/length |

The semantics of micro-instructions (with the range of possible valid codes 00-55 octal), deduced so far:

| Code | Meaning |
| --- | --- |
| 00 | NOP (conditional mark) |
| 01 | Position the iterator at the beginning (to the zero key entry;<br>needs 04 to get to the actual first element) |
| 02 | Position the iterator at the end |
| 03 | Step back (with conditionality, see 14); reaches the zero key before failing | 
| 04 | Step forward (with conditionality, see 14) |
| 05 | ??? Wants to write to the disk |
| 06 | ??? Wants to write to the disk |
| 07 | ??? Non-writing |
| 10 | Initialize a container/catalog |
| 11 | Find an entry by key |
| 12 | Copy the value to the catalog descriptor |
| 13 | Compute remaining usable space |
| 14 | If the current entry does not match the given key:<br>if the 14 op is the last in the instruction word or is followed by a non-00 instruction, error out,<br>otherwise skip the 00 insn and 3 following insns |
| 15 | The converse of 14 (if the current entry does match  the given key ....) |
| 16 | Compute character string length | 
| 17 | Compute data length in words using bdvec[32] as delimiter, buggy | 
| 20 | Update the value part of the current entry, reallocating if the size changes |
| 21 | Allocate a memory block for data or metadata | 
| 22 | Copy the value part of the current entry to the user area |
| 23 | Free the memory for the value part of the current entry |
| 24 | Password check (the 3rd word of the current file descriptor),<br>if created with 3 words instead of default 2 in NEWD |
| 25 | Switch to the catalog pointed to by the descriptor |
| 26 | Add a key |
| 27 | Delete a key |
| 30 | Restart executing the instruction word |
| 31 | Switch to the root catalog |
| 32 | [aitem] = bdvec[10]; ??? mark buffer modified |
| 33 | Compute length of the object pointed to by the descriptor |
| 34 | Converts a block descriptor to a text format (length, date) |
| 35 | Save dirty buffers and exit |
| 36 | ??? Wants to write to the disk |
| 37 | Skip next 2 instructions |
| 40 | NOP |
| 41 | Compare the user data to the current entry and skip next 2 instructions if no match |
| 42 | Update the value of the current entry in place |
| 43 | Copy a single extent of data to the user area ??? |
| 44 | bdvec[035] (address of descriptor) = bdvec[041] (first word of the copy of the descriptor) ??? |
| 45 | Setting the flag "database is busy" |
| 46 | ??? Non-writing |
| 47 | Execute a callback ??? |
| 50 | orgcmd := bdvec[bdvec[next_insn]++] (chaining; unclear which locations are available) |
| 51 | myloc := bdvec[bdvec[next_insn]++] |
| 52 | bdvec[next_insn] := bdvec[bdvec[next_next_insn]++] ??? |
| 53 | bdvec[next_insn] := bdvec[next_next_insn] ??? |
| 54 | mem[bdvec[next_insn]] := bdvec[012] ??? |
| 55 | Immediate exit |
