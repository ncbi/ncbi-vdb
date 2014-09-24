#!/usr/bin/perl -w

use IO::File;

#
# this script has to be fed from vdb-dump
# it takes input on STDIN
# the input has to be a stream of lines
# each line containing "SPOT_ID,NAME,SPOT_LEN,READ,QUALITY" delimited by tab
#
# example:
#
# vdb-dump SRR000001 -C "SPOT_ID,NAME,SPOT_LEN,READ,(INSDC:quality:text:phred_33)QUALITY" -f tab | simplefastq.pl SRR000001 > SRR000001.fastq
#

my $line;
my $base =  $ARGV[ 0 ];

if ( !defined ( $base ) )
{
    $base = "out";
}

while ( defined ( $line = <STDIN> ) )
{
    #remove line-feeds, white space etc.
    chomp( $line );
    @tokens = split( /\s/, $line );

    print( '@' );
    print( "$base.$tokens[0] $tokens[1] length=$tokens[2]\n" );
    print( "$tokens[3]\n" );
    print( "+$base.$tokens[0] $tokens[1] length=$tokens[2]\n" );
    print( "$tokens[4]\n" );
}