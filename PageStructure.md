# MARS Page Structure

All pages of the database have the same basic structure.

| Offset | Contents |            Structure (bits)             |
|:------:| ---------| --------------------------------------- |
|   0    | Zone key | <ul><li>10-1: zone number<li>48-11: value 2 for the root catalog [^1], or bits 31-1 of the file name</ul> |
|   1    |  Allocs  | <ul><li>10-1: the last unused word (L)<li>20-11: number of extents (E)</ul> |
| 2 &mdash; E+1 | Extent handles | <ul><li>10-1: start word minus 1<li>20-11: length<li>39-21: next extent in datum<li>48-40: extent number</ul>
| E+2 &mdash; L | Available | L < E+2 means "no space available"  |
| L+1 &mdash; 1777 | Extent contents | Unless L == 1777, of course |

[^1]: Using the number 2 as the file name may be "magic".
