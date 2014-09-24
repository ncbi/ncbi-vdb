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

use constant MAX_SCORE_COUNT => 64;
use constant MAX_READ_COUNT => 2;
use Data::Dumper;

my %opts = (
    'row-range' => '-1000000',
);

sub usage()
{
    print <<"HELP";
report QUALITY statistics for an SRA

Usage:
$0 [<options>...] <accession>...
options are
    -h | -? | --help
    --row-range <row-range-spec>    default is first 1000000 spots

Example:
    $0 --row-range "5,7,11-15,25-37" SRR797646

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
my $max_score = 0;

sub split_record($$$$)
{
    my @fld = \split(/\t/, ${$_[0]});
    @{$_[1]} = split(/,\s*/, ${$fld[0]});
    @{$_[2]} = split(/,\s*/, ${$fld[1]});
    @{$_[3]} = split(/,\s*/, ${$fld[2]});
    
    return [ split(/,\s*/, ${$fld[3]}) ];
}

sub resize_stats($)
{
    my $need = scalar(@{$_[0]}) * MAX_SCORE_COUNT * MAX_READ_COUNT;
    my $have = scalar(@stats);
    
    return unless ($have < $need);
    splice @stats, $have, 0, map { 0 } (1..($need - $have));
}

sub expand_and_summarize($)
{
    my @rs;
    my @rl;
    my @rt;
    my $qv = split_record $_[0], \@rs, \@rl, \@rt;
    my $reads = scalar @rs;

    die "inconsistent data" unless scalar(@rl) == $reads;
    die "inconsistent data" unless scalar(@rt) == $reads;

    resize_stats $qv;
    $max_read = $reads if $max_read < $reads;
    for (my $read = 0; $read != $reads; ++$read) {
        next unless $rt[$read] =~ /SRA_READ_TYPE_BIOLOGICAL/;
        my $pos = $rs[$read];
        my $len = $rl[$read];
        
        $max_len = $len if ($max_len < $len);
        for (my $i = 0; $i != $len; ++$i) {
            my $score = $qv->[$pos + $i] + 0;
            $score = MAX_SCORE_COUNT - 1 if $score >= MAX_SCORE_COUNT;
            my $idx = ($i * MAX_SCORE_COUNT + $score) * MAX_READ_COUNT + $read;
            
            $max_score = $score if $max_score < $score;
            ++$stats[$idx];
        }
    }
}

sub warn_if_aligned($)
{
    return unless `$VDB_DUMP -y "$_[0]"` =~ /Database/i;
    open CMD, '-|', "$VDB_DUMP -E \"$_[0]\"" or die "$!";

    while (defined(local $_ = <CMD>)) {
        chomp;
        if (/PRIMARY_ALIGNMENT/) {
            print "$_[0] is aligned; results may be biased to the reference\n";
            last;
        }
    }
    close CMD;
}

sub process($)
{
    my $cmd = "$VDB_DUMP -f tab -C \"READ_START,READ_LEN,READ_TYPE,QUALITY\" -R $opts{'row-range'} \"$_[0]\"";
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
        usage();
    }
    warn_if_aligned $_;
    process $_;
}

print join("\t", ('Read', 'Pos', 'Min', '25%', '50%', '75%', 'Max', 'Avg', 'St.Dev.', 'Mode'))."\n";

sub partition($$$)
{
    my ($array, $cnt, $le) = @_;
    my $N = scalar(@{$array});
    my $ge = 1.0 - $le;
    my $i;
    my $rsum = 0;
    
    for ($i = 0; $i != $N; ++$i) {
        next if $i == 2;
        my $n = $array->[$i];
        
        next unless $n;
        
        my $n_le = $rsum + $n;
        my $n_ge = $cnt - $rsum + $n;

        $rsum = $n_le;
        return $i if ($n_le/$cnt >= $le && $n_ge/$cnt >= $ge);
    }
    return undef;
}

for (my $read = 0; $read != $max_read; ++$read) {
    for (my $pos = 0; $pos != $max_len; ++$pos) {
        my $sum = 0;
        my $ssum = 0;
        my $cnt = 0;
        my $most = -1;
        my @part = ( MAX_SCORE_COUNT, 0, 0, 0, 0 );
        my @mode = ();
        my $array = [ map { ($stats[($pos * MAX_SCORE_COUNT + $_) * MAX_READ_COUNT + $read] || 0) } (0..$max_score) ];
        
        for (my $i = 0; $i != $max_score + 1; ++$i) {
            my $n = $array->[$i];

            $most = $n if ($most < $n);
            if ($i != 2) {
                my $x = $n * $i;
                
                $cnt += $n;
                $sum += $x;
                $ssum += $i * $x;
            }
        }
        if ($cnt) {
            my $i;
            
            for ($i = 0; $i != $max_score + 1; ++$i) {
                my $n = $array->[$i];
            
                if ($n) {
                    push @mode, $i if ($n == $most);
                    if ($i != 2) {
                        $part[0] = $i if $part[0] > $i;
                        $part[4] = $i if $part[4] < $i;
                    }
                }
            }
            $part[1] = partition $array, $cnt, 0.25;
            $part[2] = partition $array, $cnt, 0.5;
            $part[3] = partition $array, $cnt, 0.75;
            printf join("\t", ($read + 1, $pos + 1, @part, $sum/$cnt, sqrt(($ssum - $sum*$sum/$cnt)/$cnt), @mode))."\n";
        }
    }
}
