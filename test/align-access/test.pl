#!/usr/bin/perl -w

use strict;
use warnings;
use Time::HiRes;
use File::Spec;

my $testfile = "/netmnt/traces04/giab05/ftp/data/AshkenazimTrio/HG002_NA24385_son/NIST_HiSeq_HG002_Homogeneity-10953946/NHGRI_Illumina300X_AJtrio_novoalign_bams/HG002.GRCh38.300x.bam";
my $chrom = "chr1";
my $chrLength = 248956422;

chomp (my $samtools = `which samtools`);
chomp (my $tester = `which test-align-access`);

die "add samtools to PATH\n" unless $samtools;
die "add test-align-access to PATH\n" unless $tester;
die "can't read test file $testfile\n" unless -r $testfile;
die "can't read test index $testfile.bai\n" unless -f "$testfile.bai";

sub gold($$)
{
    exec {$samtools} $samtools, 'view', $testfile, "$chrom:$_[0]-$_[1]";
    die "can't exec samtools";
}

sub lead($$)
{
    exec {$tester} $tester, $testfile, $chrom, @_;
    die "can't exec $tester";
}

sub getGoldCount($$)
{
    my $kid = open(CHILD, '-|'); die "can't fork" unless defined $kid;
    if ($kid == 0) {
        goto &gold;
    }
    my $count = 0;
    my ($first, $last);
    while (defined(local $_ = <CHILD>)) {
        my @F = \split(/\t/);
        next if ((0+${$F[1]}) & 0x0004) != 0;
        next if (''.${$F[2]}) eq '*';
        next if (0+${$F[3]}) == 0;
        next if (''.${$F[5]}) eq '*';
        $last = 0+${$F[3]};
        $first = $first || $last;
        ++$count;
    }
    close CHILD;
    return ($count, $first || '0', $last || '0');
}

sub getTestCountAndTime($$)
{
    my $kid = open(CHILD, '-|'); die "can't fork" unless defined $kid;
    if ($kid == 0) {
        goto &lead;
    }
    my $start = Time::HiRes::time;
    chomp(my $out = <CHILD>);
    my $end = Time::HiRes::time;
    close CHILD;
    die unless defined $out;
    my ($count, $first, $last) = (split(/\s+/, $out))[-3..-1];
    return ($count, $end - $start, $first, $last);
}

sub getGoldTime($$)
{
    my $kid = fork(); die "can't fork" unless defined $kid;
    if ($kid == 0) {
        open STDOUT, '>', File::Spec->devnull();
        goto &gold;
    }
    my $start = Time::HiRes::time;
    waitpid($kid, 0);
    my $end = Time::HiRes::time;
    die "samtools died with ".($?>>8)."\n" if $?;
    return $end - $start;
}

my @tests;
for (1..500) {
    my $beg;
    my $end;
    do {
        $beg = int(rand($chrLength));
        $end = $beg + int(rand(100_000));
    } while ($beg == $end || $end > $chrLength);
    push @tests, { beg => $beg, end => $end };
}
my @timeDiff;

for (sort { my $diff1 = $a->{beg} - $b->{beg}; return $diff1 == 0 ? $a->{end} - $b->{end} : $diff1 } @tests) {
    my $beg = $_->{beg}+1;
    my $end = $_->{end};
    my ($goldCount, $goldFirst, $goldLast) = getGoldCount($beg, $end);
    my ($testCount, $testTime, $testFirst, $testLast) = getTestCountAndTime($beg, $end);
    if ($goldCount == $testCount && $goldFirst == $testFirst && $goldLast == $testLast) {
        my $goldTime = getGoldTime($beg, $end);
        push @timeDiff, $testTime / $goldTime;
        next;
    }
    my $bad = ($goldFirst == $testFirst && $goldLast == $testLast) ? '' : '*';
    print join("\t", $beg, $end, $goldCount, $testCount, $goldFirst, $testFirst, $goldLast, $testLast, $bad)."\n";
}

if (@timeDiff) {
    my $N = scalar(@timeDiff);
    my $sum = 0;
    $sum += $_ for @timeDiff;

    my $avg = $sum / $N;

    $sum = 0;
    $sum += ($_ - $avg) * ($_ - $avg) for @timeDiff;

    my $stdev = sqrt($sum / $N);

    printf "Average: %f; StDev: %f\n", $avg, $stdev;
    for (@timeDiff) {
        my $t;
        if ($_ < $avg) {
            $t = ($avg - $_) / $stdev;
        }
        else { 
            $t = ($_ - $avg) / $stdev;
        }
        print "$_\n" if $t > 2.0;
    }
}
