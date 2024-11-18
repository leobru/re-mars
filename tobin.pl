#!/usr/bin/perl
use strict;
use warnings;

# Open output file for binary data
my $output_file = "output.bin";
open(my $out, '>:raw', $output_file) or die "Could not open $output_file: $!";

while (<STDIN>) {
    next unless /:  (....) (....) (....) (....)/;

    # Convert octal to decimal
    my $decimal = oct($1) << 36 | oct($2) << 24 | oct($3) << 12 | oct($4);

    # Pack as a 64-bit binary value (unsigned, big-endian)
    my $binary = pack("Q<", $decimal);

    # Write the binary value to the file
    print $binary;
}
