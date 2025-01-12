# MARS-6 archive system

MARS-6 was an implementation of an external sorted key-value container.
The API mechanism was in form of a micro-program.

Pascal compilers for the BESM-6 included a few MARS-6 operations as part ot the standard library, encoded as micro-programs.
They are:

| Operation | Micro-program | Comment |
| :-------: | :-----------: | --- |
| GETD  | FIND MATCH GET | Get the value by key, error out if the key does not exist |
| PUTD  | FIND NOMATCH ALLOC ADDKEY | Add a key-value pair, error out if the key already exists |
| DELD  | FIND MATCH FREE DELKEY | Delete a pair by key, error out if the key does not exist |
| MODD  | FIND NOMATCH (COND ALLOC ADDKEY STOP) UPDATE | Modify the value if a pair with the key already exists, add a pair otherwise |
| OPEND | ROOT FIND MATCH SETCTL OPEN | Open an existing container by name, error out if it does not exist in the root catalog |
| NEWD  | ROOT FIND NOMATCH ALLOC ADDKEY<br>FIND MATCH SETCTL INIT | Create a container given a name and LUN/starting block/length |

Below are micro-programs found in the binary code of a program which used the MARS-6 system. Not all are fully understood.

| Micro-program | Comment |
| :-----------: | --- |
| FIND MATCH (COND FREE DELKEY NOP) ALLOC ADDKEY | An alternative implementation of MODD,<br>deleting the key before inserting the new value |
| ROOT FIND MATCH SETCTL OPEN AVAIL BEGIN | Open a container, compute available space,<br>position the iterator to the starting dummy entry |
| FIND MATCH SETMETA | CHDIR: If the key-value pair exists, set the value as the main metablock , otherwise error out |
| FIND NOMATCH INSMETA ADDKEY | MKDIR: If the key does not exist, add a key-metablock pair, otherwise error out |
| BEGIN SETMETA | DOTDOT: CHDIR to the parent directory (no op if the current directory is the root) |
| BEGIN NEXT | Position the iterator to the first real entry |
| LAST NOMATCH (COND FREE DELKEY LOOP) DELKEY | Deletion in the loop (backwards, this is faster);<br>down to and including the entry with the specified key (must not be 0) |
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
|  07  |   SEEK   | Seek to the given positive offset relative to the current position within the current datum  |
|  10  |   INIT   | Initialize a container/catalog |
|  11  |   FIND   | Find an entry by key |
|  12  |  SETCTL  | Copy the value to the catalog descriptor |
|  13  |   AVAIL  | Compute remaining usable space |
|  14  |   MATCH  | If the current entry does not match the given key:<br>if the 14 op is the last in the instruction word or is followed by a non-00 instruction, error out,<br>otherwise skip the 00 insn and 3 following insns |
|  15  |  NOMATCH | The converse of 14 (if the current entry does match  the given key ....) |
|  16  |  STRLEN  | Compute character string length |
|  17  |  WORDLEN | Compute data length in words using bdvec[32] as delimiter, buggy |
|  20  |  UPDATE  | Update the value part of the current entry, reallocating if the size changes |
|  21  |  ALLOC   | Allocate a memory block for data or metadata |
|  22  |    GET   | Copy the value part of the current entry to the user area |
|  23  |   FREE   | Free the memory for the value part of the current entry |
|  24  |  PASSWD  | Password check (against the 3rd word of the current file descriptor),<br>if created with 3 words instead of default 2 in NEWD |
|  25  |   OPEN   | Switch to the catalog pointed to by the descriptor |
|  26  |  ADDKEY  | Add a key |
|  27  |  DELKEY  | Delete a key |
|  30  |   LOOP   | Restart executing the instruction word |
|  31  |   ROOT   | Switch to the root catalog |
|  32  |  INSERT  | Inserts the latest allocated datum descriptor into the current datum position |
|  33  |  LENGTH  | Compute length of the object pointed to by the descriptor |
|  34  |   DESCR  | Converts a block descriptor to a text format (length, date) |
|  35  |   SAVE   | Save dirty buffers and exit |
|  36  |  ADDMETA | Adds the current descriptor to the current metablock position,<br>updates the metablock tree |
|  37  |   SKIP   | Skip the next instruction |
|  40  |   STOP   | Stops the execution of the micro-program |
|  41  |   IFEQ   | Compare the user data to the current entry and skip next 2 instructions if no match |
|  42  |   WRITE  | Modify the value of the current datum in place |
|  43  |   READ   | Copy a word range of the current datum to the user area |
|  44  |    USE   | Sets the word BDVECT[041] (assigned by SEEK) as the current entry pointer |
|  45  |   LOCK   | Setting the flag "database is busy", unlocked at the end of execution of the microprogram |
|  46  |  UNPACK  | Unpacks myloc into length and offset to be followed by SEEK and READ/WRITE for scatter/gather<br>Skips 4 following instructions if length == 0 |
|  47  |   CALL   | Execute a callback ??? |
|  50  |   CHAIN  | orgcmd := mem[bdvec[next_insn]++] (chaining) |
|  51  |  SEGMENT | myloc := mem[bdvec[next_insn]++] for scatter/gather |
|  52  |  LDNEXT  | bdvec[next_insn] := mem[bdvec[next_next_insn]++] |
|  53  |  ASSIGN  | bdvec[next_insn] := bdvec[next_next_insn]  |
|  54  |  STALLOC | mem[bdvec[next_insn]] := BDVECT[012] (assigned by ALLOC) |
|  55  |   EXIT   | Immediate exit |

BDVECT inferface structure:

| Offset |                      Meaning                 |
| :----: | -------------------------------------------- |
|   0    | Address of the next instruction word         |
|   1    | Function pointer for CALL                    |
|   2    |                                              |
|   3    | Initial instruction word                     |
|   4    |                                              |
|   5    | Current instruction word                     |
|   6    | DB sync disable flags                        |
|   7    | Password to be checked by PASSWD             |
|  10    | Key for FIND                                 |
|  11    | Error handler function pointer               |
|  12    | Datum handle returned by ALLOC               |
|  13    | Pointer to user data, etc. (used by UNPACK)  |
|  14    | Extra field for datum header                 |
|  15    | Length of user data                          |
|  16    | Pointer to the BDBUF page                    |
|  17    | Pointer to the BDTAB page                    |
|  20-27 | Cursor                                       |
|  30-32 | DB file descriptor (location, key, password) |
|  33    | (BESM-6 relocation register to be restored)  |
|  34    | Pointer to the free space array within BDTAB |
|  35    | Datum handle operated upon (set by FIND, etc)|
|  36    | (Temp. variable for ADDKEY)                  |
|  37    | Key of the entry pointed by the cursor       |
|  40    | End marker for STRLEN and WORDLEN            |
|  41    | Current datum word (set by SEEK), etc.       |
|  42    | Offset for SEEK, etc.                        |
|  43    |                                              |
|  44    | (Temp. variable for I/O sys calls)           |
|  45    | Length of the current extent                 |
|  46    | Pointer to the current datum word            |
|  47    | Length of the current datum (set by LENGTH)  |
|  50    | Pointer to the current metablock             |
|  51    | DB length in zones                           |
|  52    |                                              |
|  53    | Current position within the datum            |
|  54-115| Root metablock                               |
| 116-217| Secondary metablock; 116 also used as temp.  |
| 220    | Current extent                               |
| 221-240|                                              |
|  241   | Pointer to the current buffer (BDTAB/BDBUF)  |
|  242   | Index into the cursor                        |
|  243   |                                              |
|  244   | Current zone number                          |
|  245   |                                              |
|  246   | Handle of the current metablock              |
|  247   | Dirty buffer flags                           |