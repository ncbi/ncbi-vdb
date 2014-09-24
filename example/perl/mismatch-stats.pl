#!/usr/bin/env perl
# ===========================================================================
#
#                            PUBLIC DOMAIN NOTICE
#               National Center for Biotechnology Information
#
#  This software/database is a "United States Government Work" under the
#  terms of the United States Copyright Act.  It was written as part of
#  the author's official duties as a United States Government employee and
#  thus cannot be copyrighted.  This software/database is freely available
#  to the public for use. The National Library of Medicine and the U.S.
#  Government have not placed any restriction on its use or reproduction.
#
#  Although all reasonable efforts have been taken to ensure the accuracy
#  and reliability of the software and data, the NLM and the U.S.
#  Government do not and cannot warrant the performance or results that
#  may be obtained by using this software or data. The NLM and the U.S.
#  Government disclaim all warranties, express or implied, including
#  warranties of performance, merchantability or fitness for any particular
#  purpose.
#
#  Please cite the author in any work or product based on this material.
#
# ===========================================================================

use warnings;

use Data::Dumper;

use constant MAX_READ_COUNT => 2;

my %opts = (
    'row-range' => '-1000000',
    'output-type' => 'proportion'
);

sub usage()
{
    print <<"HELP";
report BASE statistics for an SRA

Usage:
$0 [<options>...] <accession>...
options are
    -h | -? | --help
    --row-range <row-range-spec>    default is first 1000000 spots
    --output-type <output-spec>     set the type of data output 
                                        count, percent, or proportion
                                        proportion is the default

Example:
    $0 --row-range "5,7,11-15,25-37" --output-type count SRR797646

HELP
    exit 0;
}

usage if scalar @ARGV == 0;
foreach (@ARGV) {
    usage() if (/^-h$/ || /^-\?$/ || /^--help$/);
}

my $VDB_DUMP = `which vdb-dump` or die "Please put path to vdb-dump in PATH";
chomp $VDB_DUMP;

my @stats = ();
my $max_read = 0;
my $max_len = 0;

sub get_index($$$)
{
    my ($pos, $read, $base) = @_;
    
    return ($pos * MAX_READ_COUNT + $read) * 6 + $base;
}

sub resize_stats($)
{
    my $need = get_index $_[0], MAX_READ_COUNT, 0;
    my $have = scalar(@stats);
    
    return unless ($have < $need);
    splice @stats, $have, 0, map { 0 } (0..($need - $have));
}

sub translate_base($)
{
    return 0 if $_[0] eq '=';
    return 1 if $_[0] eq 'A';
    return 2 if $_[0] eq 'C';
    return 3 if $_[0] eq 'G';
    return 4 if $_[0] eq 'T';
    return 5;
}

sub translate_base_complement($)
{
    return 0 if $_[0] eq '=';
    return 1 if $_[0] eq 'T';
    return 2 if $_[0] eq 'G';
    return 3 if $_[0] eq 'C';
    return 4 if $_[0] eq 'A';
    return 5;
}

sub expand_and_summarize($)
{
    my ($READ_ID, $ORIENT, $MM_READ) = \split(/\t/, ${$_[0]});
    my $len = length $$MM_READ;
    my $read = $$READ_ID - 1;
    
    $max_read = $read if $max_read < $read;
    $max_len  = $len  if $max_len  < $len ;
    
    resize_stats $len;
    if ($$ORIENT eq 'false') {
        for (my $i = 0; $i != $len; ++$i) {
            my $base = translate_base(substr($$MM_READ, $i, 1));
    
            ++$stats[get_index($i, $read, $base)];
        }
    }
    else {
        for (my $j = 0; $j != $len; ++$j) {
            my $i = $len - $j - 1;
            my $base = translate_base_complement(substr($$MM_READ, $i, 1));
    
            ++$stats[get_index($i, $read, $base)];
        }
    }
}

sub check_aligned($)
{
    my $aligned = 0;
    
    if (`$VDB_DUMP -y "$_[0]"` =~ /Database/i) {
        open CMD, '-|', "$VDB_DUMP -E \"$_[0]\"" or die "$!";

        while (defined(local $_ = <CMD>)) {
            chomp;
            if (/PRIMARY_ALIGNMENT/) {
                $aligned = 1;
                last;
            }
        }
        close CMD;
    }
    return $aligned;
}

sub process($)
{
    my $cmd = "$VDB_DUMP -f tab -C \"SEQ_READ_ID,REF_ORIENTATION,MISMATCH_READ\" -R $opts{'row-range'} -T PRIMARY_ALIGNMENT \"$_[0]\"";
    open CMD, '-|', $cmd or die "$!";

    while (defined($_ = <CMD>)) {
        chomp;
        expand_and_summarize \$_;
    }
    close CMD;
}

for (my $i = 0; $i < scalar @ARGV; ++$i) {
    $_ = $ARGV[$i];

    if (/^-/) {
        if (/^--row-range$/) {
            $opts{'row-range'} = $ARGV[++$i];
            next;
        }
        if (/^--output-type$/) {
            $opts{'output-type'} = $ARGV[++$i];
            next;
        }
        usage();
    }
    if (check_aligned $_) {
        process $_;
    }
    else {
        print "$_ is not aligned\n";
    }
}

sub format_counts($$$$)
{
    my ($x, $cnt) = @_;
    
    return $x;
}

sub format_proportion($$$$)
{
    my ($x, $cnt) = @_;
    
    return $x/$cnt;
}

sub format_percent($$$$)
{
    my ($x, $cnt) = @_;
    
    return int($x * 100.0/$cnt + 0.5).'%';
}

my $format = \&format_proportion;

if ($opts{'output-type'} eq 'count') {
    $format = \&format_counts;
}
elsif ($opts{'output-type'} eq 'proportion') {
    $format = \&format_proportion;
}
elsif ($opts{'output-type'} eq 'percent') {
    $format = \&format_percent;
}
else {
    usage();
}

printf join("\t", ('Read', 'Pos', '=', 'A', 'C', 'G', 'T', 'N'))."\n";

for (my $read = 0; $read <= $max_read; ++$read) {
    for (my $pos = 0; $pos != $max_len; ++$pos) {
        my $idx = get_index $pos, $read, 0;
        my $tot = $stats[$idx+0] + $stats[$idx+1] + $stats[$idx+2]
                + $stats[$idx+3] + $stats[$idx+4] + $stats[$idx+5];
        
        if ($tot) {
            print join("\t", ($read + 1, $pos + 1,
                &$format($stats[$idx+0], $tot),
                &$format($stats[$idx+1], $tot),
                &$format($stats[$idx+2], $tot),
                &$format($stats[$idx+3], $tot),
                &$format($stats[$idx+4], $tot),
                &$format($stats[$idx+5], $tot)
                ))."\n";
        }
    }
}
