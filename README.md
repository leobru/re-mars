# MARS-6 archive system

MARS-6 was an implementation of an external sorted key-value container.
The API mechanism was in form of a micro-program.

Pascal compilers for the BESM-6 included a few MARS-6 operations as part ot the standard library, encoded as micro-programs.
They are:

| Operation | Micro-program | Comment |
| :-------: | :-----------: | --- |
| GETD  | FIND MATCH GET | Get the value by key, error out if the key does not exist |
| PUTD  | FIND NOMATCH INSERT ADDKEY | Add a key-value pair, error out if the key already exists |
| DELD  | FIND MATCH FREE DELKEY | Delete a pair by key, error out if the key does not exist |
| MODD  | FIND NOMATCH (COND INSERT ADDKEY STOP) UPDATE | Modify the value if a pair with the key already exists, add a pair otherwise |
| OPEND | ROOT FIND MATCH SETCTL OPEN | Open an existing container by name, error out if it does not exist in the root catalog |
| NEWD  | ROOT FIND NOMATCH INSERT ADDKEY<br>FIND MATCH SETCTL INIT | Create a container given a name and LUN/starting block/length |

Below are micro-programs found in the binary code of a program which used the MARS-6 system. Not all are fully understood.

| Micro-program | Comment |
| :-----------: | --- |
| FIND MATCH (COND FREE DELKEY NOP) INSERT ADDKEY | An alternative implementation of MODD,<br>deleting the key before inserting the new value |
| ROOT FIND MATCH SETCTL OPEN AVAIL BEGIN | Open a container, compute available space,<br>position the iterator to the starting dummy entry |
| FIND MATCH SETMETA | If the key-value pair exists, set the value as the main metablock, otherwise error out |
| FIND NOMATCH INSMETA ADDKEY | If the key does not exist, add a key-metablock pair, otherwise error out |
| BEGIN SETMETA | Reset the metablock to the root ??? |
| BEGIN NEXT | Position the iterator to the first real entry |
| LAST NOMATCH (COND FREE DELKEY LOOP) DELKEY | Deletion in the loop (backwards, this is faster);<br>the last DELKEY looks erroneous |
| ROOT FIND MATCH | Check that a file exists; do not open |
| LAST COND FREE DELKEY LOOP | Deletion in the loop; COND is useless, as LAST is not conditional;<br>will error out and potentially corrupt the DB |
| FIND FREE DELKEY | Unconditional deletion |

The semantics of micro-instructions (with the range of possible valid codes 00-55 octal), deduced so far:

| Code | Mnemonic |       Meaning          |
| :--: | :-----: | ---------------------- |
|  00  | NOP/COND | no-op/conditional mark |
|  01  |   BEGIN  | Position the iterator at the beginning (to the zero key entry;<br>needs 04 to get to the actual first element) |
|  02  |   LAST   | Position the iterator at the end |
|  03  |   PREV   | Step back (with conditionality, see 14); reaches the zero key before failing |
|  04  |   NEXT   | Step forward (with conditionality, see 14) |
|  05  |  INSMETA | Makes and inserts a metadata block |
|  06  |  SETMETA | Sets the current datum as the main metadata block  |
|  07  |    ???   | Non-writing |
|  10  |   INIT   | Initialize a container/catalog |
|  11  |   FIND   | Find an entry by key |
|  12  |  SETCTL  | Copy the value to the catalog descriptor |
|  13  |   AVAIL  | Compute remaining usable space |
|  14  |   MATCH  | If the current entry does not match the given key:<br>if the 14 op is the last in the instruction word or is followed by a non-00 instruction, error out,<br>otherwise skip the 00 insn and 3 following insns |
|  15  |  NOMATCH | The converse of 14 (if the current entry does match  the given key ....) |
|  16  |  STRLEN  | Compute character string length |
|  17  |  WORDLEN | Compute data length in words using bdvec[32] as delimiter, buggy |
|  20  |  UPDATE  | Update the value part of the current entry, reallocating if the size changes |
|  21  |  INSERT  | Allocate a memory block for data or metadata |
|  22  |    GET   | Copy the value part of the current entry to the user area |
|  23  |   FREE   | Free the memory for the value part of the current entry |
|  24  |  PASSWD  | Password check (against the 3rd word of the current file descriptor),<br>if created with 3 words instead of default 2 in NEWD |
|  25  |   OPEN   | Switch to the catalog pointed to by the descriptor |
|  26  |  ADDKEY  | Add a key |
|  27  |  DELKEY  | Delete a key |
|  30  |   LOOP   | Restart executing the instruction word |
|  31  |   ROOT   | Switch to the root catalog |
|  32  |    ???   | *extPtr = curDescr; ??? mark buffer modified |
|  33  |  LENGTH  | Compute length of the object pointed to by the descriptor |
|  34  |   DESCR  | Converts a block descriptor to a text format (length, date) |
|  35  |   SAVE   | Save dirty buffers and exit |
|  36  |  ADDMETA | Adds the current descriptor to the current metablock position,<br>updates the metablock tree |
|  37  |   SKIP   | Skip the next instruction |
|  40  |   STOP   | Stops the execution of the micro-program |
|  41  |   IFEQ   | Compare the user data to the current entry and skip next 2 instructions if no match |
|  42  |  MODIFY  | Modify the value of the current data extent in place |
|  43  |   COPY   | Copy a single extent of data to the user area ??? |
|  44  |    ???   | bdvec[035] (address of descriptor) = bdvec[041] (first word of the copy of the descriptor) |
|  45  |   LOCK   | Setting the flag "database is busy", unlocked at the end of execution of the microprogram |
|  46  |    ???   | Non-writing |
|  47  |   CALL   | Execute a callback ??? |
|  50  |   CHAIN  | orgcmd := mem[bdvec[next_insn]++] (chaining) |
|  51  |    ???   | myloc := mem[bdvec[next_insn]++] |
|  52  |    ???   | bdvec[next_insn] := mem[bdvec[next_next_insn]++] |
|  53  |  ASSIGN  | bdvec[next_insn] := bdvec[next_next_insn]  |
|  54  |    ???   | mem[bdvec[next_insn]] := curDescr |
|  55  |   EXIT   | Immediate exit |
