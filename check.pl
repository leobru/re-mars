#!/usr/bin/env perl
die "Usage: check.pl <N zones (oct)>\n" unless $#ARGV == 0;
$nzon = oct($ARGV[0]);
chomp(@gold = `besmtool dump 1234 --start=0 --length=$nzon | cut -b 1-31 | grep -v Zone`);

for ($z = 0; $z < $nzon; ++$z) {
$fname = sprintf('52%04o.txt', $z);
open(F, $fname) || die "$fname does not exist\n";
for ($i = 0; $i < 1024; ++$i) {
chomp ($word = <F>);
next if $word =~ /1234 5670 0765 4321/;
$good = $gold[$z*1024+$i];
print "$good <- gold silver -> $word\n" unless $good eq $word;
}
}
