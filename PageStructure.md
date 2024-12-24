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

Extent 1 of zone 0 is the root metablock of length 041; extent 2 of zone 0 is the free space table, one word per zone.
Therefore, the maximum DB size is 02000 - 2 (header) - 2 (extent handles) - 2 (extent headers) - 041 (root metablock) = 01731 zones, slightly greater than 1&#x23e8;6 words.

The 19-bit extent locations are (9-bit ID, 10-bit zone).

## Extent Handle

| Bits | Contents |
|:----:| ---------|
|48-40 | Extent ID |
|39-21 | 0 - this is the last extent in datum<br> >0 - next extent in datum |
|20-11 | Extent length |
|10-1  | Start word minus 1 |

## Extent Header

| Bits | Contents |
|:----:| ---------|
|48-34 | Date stamp, DDMMY in compressed BCD (2-4-1-4-4 bits) format |
|33-16 | Can be set from a field in the BDVECT structure, use unclear |
| 15-1 | Total length of the datum |

Continuation extents of fragmented data do not have headers.

## Metablock

| Offset | Contents | Structure (bits) |
|:------:|----------|------------------|
|   0    | Header   |  <ul><li>48-30: Next block<li>29-11: Previous block<li>10-1: Words used (always an even number)</ul> |
|  2k-1  | Key      |  <ul><li>48: 0 (to ensure stability of comparison using cyclic addition)<li>47-1: Key</ul> |
|  2k    | Location | <ul><li>48: indirect flag<li>19-1: extent location</ul> |
