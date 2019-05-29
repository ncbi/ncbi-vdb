#!/usr/bin/perl -w
my ($x, $c) = (shift, shift);
die unless $c =~ /^\d+$/;
my $s = system($x);
die if $s == -1;
exit 1 unless $c == ($s >> 8);
exit 0;
