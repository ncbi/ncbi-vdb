#!/usr/bin/env perl
# =============================================================================
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
# =============================================================================

=head1 NAME

base-stats - report base statistics for an SRA run

=head1 DESCRIPTION

This script computes base and GC content statistics for an SRA run.  The output
is summarized by mate and position within the mate.

=head1 PURPOSE

This script demonstrates driving the SRA Toolkit tool 'vdb-dump' from Perl for
accessing data in the SRA.  It requires that the toolkit be properly configured.
'vdb-dump' is the Jack-of-all-trades of the SRA Toolkit.

=head1 EXAMPLE

base-stats.pl SRR797646

=head1 SEE ALSO

base-stats.pl --help

vdb-dump --help

=cut

use strict;
use warnings;

sub usage()
{
    print <<"HELP";
report base statistics for an SRA run

Usage:
$0 [<options>...] <accession>...
options are
    -h | -? | --help                shows this message
    --row-range <row-range-spec>    default is spots 1-1000000
        row-range-spec              a comma seperated list of row numbers and/or
                                    ranges; a range is <first>-<last>
                                    
    --output-type <output-spec>     set the type of data output; proportion is
                                    the default
        output-spec                 one of 'count', 'percent', or 'proportion'

    accession                       an SRA accession or path to an SRA file
    
Example:
    $0 --row-range "5,7,11-15,25-37" --output-type count SRR797646

HELP
    exit 0;
}

usage() if scalar @ARGV == 0;
foreach (@ARGV) {
    usage() if (/^-h$/ || /^-\?$/ || /^--help$/);
}

use constant MAX_READ_COUNT => 2;

# the columns to be read from the input
use constant COLUMN_LIST => 'READ_START,READ_LEN,READ_TYPE,READ';

# need vdb-dump to read input files
my $VDB_DUMP = `which vdb-dump` or die "Please put path to vdb-dump in PATH";
chomp $VDB_DUMP;

my %opts = (
    'row-range'   => '1-1000000',      # first million spots
    'output-type' => 'proportion'
);

sub foreach_row($$)
{
    my ($source, $callback) = @_;
    my $cmd = $VDB_DUMP
            . ' --rows "' . $opts{'row-range'} . '"'    # set the row range
            . ' --columns "' . COLUMN_LIST . '"'        # set the columns to be read
            . ' --format tab'                           # set output format to tab delimited
            . ' "' . $source . '"';                     # the file/accession to be read
    open CMD, '-|', $cmd or die "failed to exec vdb-dump: $!";

    while (defined(local $_ = <CMD>)) {
#       print; # uncomment to print raw output from vdb-dump (e.g. for debugging)
        chomp;

        &$callback(\split(/\t/, $_));
    }
    close CMD;
}

sub sra_object_type($)
{
    open CMD, '-|', $VDB_DUMP .' --obj_type "' . $_[0] . '"' or die "failed to exec vdb-dump: $!";
    local $_ = <CMD>;

    close CMD;
    chomp;
    return $_;
}

sub sra_list_tables($)
{
    my @rslt = ();
    
    if (sra_object_type($_[0]) eq 'Database') {
        open CMD, '-|', $VDB_DUMP . ' --table_enum "' . $_[0] . '"' or die "failed to exec vdb-dump: $!";
        while (defined(local $_ = <CMD>)) {
            push @rslt, $1 if /^tbl #\d+:\s*(\w+)/;
        }
        close CMD;
    }
    return @rslt;
}

sub warn_if_aligned($);
sub print_result();
sub process;

my @stats = ();
my $max_read = 0;
my $max_pos = 0;

for (my $i = 0; $i < scalar @ARGV; ++$i) {
    $_ = $ARGV[$i];

    if (/^-/) {
        if (/^--row-range$/) {
            $opts{'row-range'} = $ARGV[++$i];
            next;
        }
        if (/^--output-type$/) {
            my $type = $ARGV[++$i];

            if (   $type eq 'count'
                || $type eq 'proportion'
                || $type eq 'percent')
            {
                $opts{'output-type'} = $type;
                next;
            }
        }
        usage();
    }
    warn_if_aligned $_;
    foreach_row($_, \&process);
}

print_result();
exit 0;

sub warn_if_aligned($)
{
    local $_;
    
    foreach (sra_list_tables($_[0])) {
        if (/PRIMARY_ALIGNMENT/) {
            print "$_[0] is aligned and is likely to be mostly ordered; results may be biased to the reference\n";
            last;
        }
    }
}

sub get_stat($$$)
{
    # (pos * 5 + base) * MAX_READ_COUNT + read_number;
    return $stats[($_[1] * 5 + $_[2]) * MAX_READ_COUNT + $_[0]];
}

sub inc_stat($$$) # this is called a zillion times and needs to be fast
{
    # (pos * 5 + base) * MAX_READ_COUNT + read_number;
    ++$stats[($_[1] * 5 + $_[2]) * MAX_READ_COUNT + $_[0]];
}

sub resize_stats($)
{
    my $length = $_[0];
    my $need = MAX_READ_COUNT * 5 * $length;
    my $have = scalar(@stats);
    
    return if $have >= $need;
    splice @stats, $have, 0, map { 0 } (1..($need - $have));
}

sub convert_bases($)
{
    local $_ = ${$_[0]};
    
    tr/ACGTN/01234/;
    return $_;
}

# accumulate statistics for one spot
sub process
{
    # this matches COLUMN_LIST above
    my ($READ_START, $READ_LEN, $READ_TYPE, $READ) = @_;

    # the data in these fields is comma delimited
    # and describe how to split READ up into its pieces
    my @start  = split(/,\s*/, $$READ_START);
    my @length = split(/,\s*/, $$READ_LEN);
    my @type   = split(/,\s*/, $$READ_TYPE);

    # READ contains the bases for the whole spot (i.e. all the mates concatenated)
    my @bases  = split(//, convert_bases($READ));
    my $reads  = scalar(@start);

    die "too many reads: $reads reads; adjust MAX_READ_COUNT and rerun" unless $reads <= MAX_READ_COUNT;

    $max_read = $reads if ($max_read < $reads);
    for (my $read = 0; $read != $reads; ++$read) {
        # only count biological bases (i.e. no adapters, linkers, etc.)
        next unless $type[$read] =~ /SRA_READ_TYPE_BIOLOGICAL/;
        my $pos = $start[$read];
        my $len = $length[$read];

        resize_stats($len);
        $max_pos = $len if ($max_pos < $len);
        for (my $i = 0; $i != $len; ++$i) {
            inc_stat($read, $i, $bases[$pos + $i]);
        }
    }
}

sub print_result() {
    my $GCp_mean;
    my $GCp_stdev;
    my $sum = 0;
    my $ssum = 0;
    my $format =
        $opts{'output-type'} eq 'count'   ? sub { $_[0] } :
        $opts{'output-type'} eq 'percent' ? sub { int($_[0] * 100.0/$_[1] + 0.5).'%' } :
                                            sub { $_[0]/$_[1] };

    for (my $read = 0; $read != $max_read; ++$read) {
        for (my $pos = 0; $pos != $max_pos; ++$pos) {
            my $gc = (get_stat($read, $pos, 1) + get_stat($read, $pos, 2)) * 1.0;
            my $n = $gc + get_stat($read, $pos, 0) + get_stat($read, $pos, 3);
            my $x = $gc/$n;
            
            $sum += $x;
            $ssum += $x * $x;
        }
    }
    $GCp_mean = $sum/($max_pos*$max_read);
    $GCp_stdev = sqrt(($ssum - $sum*$GCp_mean)/($max_pos*$max_read));

    printf join("\t", ('Read', 'Pos', 'A', 'C', 'G', 'T', 'N', 'GC', 'Suspect'))."\n";
    for (my $read = 0; $read != $max_read; ++$read) {
        for (my $pos = 0; $pos != $max_pos; ++$pos) {
            my $cntA = get_stat($read, $pos, 0);
            my $cntC = get_stat($read, $pos, 1);
            my $cntG = get_stat($read, $pos, 2);
            my $cntT = get_stat($read, $pos, 3);
            my $cntN = get_stat($read, $pos, 4);
            my $GC   = $cntC + $cntG;
            my $tot  = $cntA + $GC + $cntT;

            # keep N out for GC
            my $GC_score = ($GC/$tot - $GCp_mean)/$GCp_stdev;
            
            # add N in
            $tot += $cntN;
            print join("\t", ($read + 1, $pos + 1,
                &$format($cntA, $tot),
                &$format($cntC, $tot),
                &$format($cntG, $tot),
                &$format($cntT, $tot),
                &$format($cntN, $tot),
                &$format($GC, $tot),
                '*'x(int(abs($GC_score)))))."\n";
        }
    }
}
