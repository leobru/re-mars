# MARS Page Structure

All pages of the database have the same basic structure.

| Offset | Contents |               Comment              |
|:------:| ---------| ---------------------------------- |
|   0    | Zone key | <ul><li>bits 10-1: zone number<li>bits 48-11: value 2 for the root catalog [^1], or bits 31-1 of the file name</ul> |
|   1    |  Allocs  | <ul><li>bits 10-1: the last unused word (L)<li>bits 20-11: number of extents (E)</ul> |
| 2 &mdash;  E+1 | Extent handles | <ul><li>its 10-1: offset of the extent header<li>bits 20-11: total extent length</ul>
| E+2 &mdash; L | Available | ALways maintained contiguous |
| L+1 &mdash; 1777 | Extent contents | Unless L == 1777, of course |

[^1]: Using the number 2 as the file name may be "magic".
