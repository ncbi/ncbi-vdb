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

my %opts = (
    'local-name' => 0,
    'line-length' => 70
);

sub usage()
{
    print <<"HELP";
extracts the reference sequence as FASTA from an aligned SRA

Usage:
$0 [<options>...] <accession>...
options are
    -h | -? | --help    this message
    --use-local         toggle def-line between reference accession and
                        original (local to run) name of reference
    --line-length N     sequence line length (use 0 for unlimited)
                        (default is $opts{'line-length'})

Example:
    $0 SRR341548 > ref.fasta

HELP
    exit 0;
}

usage if scalar @ARGV == 0;
foreach (@ARGV) {
    usage() if (/^-h$/ || /^-\?$/ || /^--help$/);
}

my $VDB_DUMP = `which vdb-dump` or die "Please put path to vdb-dump in PATH";
chomp $VDB_DUMP;

sub process($)
{
    my $defline = '';
    my $ref = '';
    
    open IN, '-|', "$VDB_DUMP -f tab -T REFERENCE -C \"NAME,SEQ_ID,READ\" \"$_[0]\"" or die "$!";
    
    while (defined($_ = <IN>)) {
        chomp;
        my ($name, $seqid, $seq) = \split /\t/;
        my $new_defline;
        
        if ($opts{'local-name'}) {
            $new_defline = ">$$name $$seqid";
        }
        else {
            $new_defline = ">$$seqid $$name";
        }
        if ($defline ne $new_defline) {
            print "$ref\n" if $ref;
            print "$new_defline\n";
            $defline = $new_defline;
            $ref = '';
        }
        $ref .= $$seq;
        if ($opts{'line-length'} != 0) {
            while (length($ref) >= $opts{'line-length'}) {
                print substr($ref, 0, $opts{'line-length'})."\n";
                $ref = substr($ref, $opts{'line-length'});
            }
        }
    }
    print "$ref\n" if $ref;
    close IN;
}

for (my $i = 0; $i < scalar @ARGV; ++$i) {
    $_ = $ARGV[$i];
    
    if (/^-/) {
        if (/^--use-local$/) {
            $opts{'local-name'} = 1 - $opts{'local-name'};
            next;
        }
        if (/^--line-length$/) {
            $_ = $ARGV[++$i];
            /^(\d+)$/ or usage();
            $opts{'line-length'} = $1;
            next;
        }
        usage();
    }
    process $_;
}
