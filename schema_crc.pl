#!/usr/bin/perl -w
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
use File::Path;
use IO::File;

# walks a given range of accessions and prints 'crc of schema', platform,
# obj-time, tabdb and loader

# example: schema_crc.pl SRR 000010 000020
# walks accessions SRR000010 ... SRR000020


walk_acc ( @ARGV );

sub walk_acc
{
    my( $prefix, $first, $last ) = @_;
    my $count;

    for ( $count = $first; $count <= $last; $count++ )
    {
        my $acc  = sprintf( "$prefix%.06d", $count );
        my $path = `srapath $acc 2>/dev/null`;
        chomp( $path );
        if ( -e "$path" )
        {
            my $crc  = get_schema_crc( $acc );
            my $pfm  = get_platform( $acc );
            my $time = get_obj_time( $path );
            my $tdb  = get_tabdb( $acc );
            my $ldr  = get_loader( $path );
            my $pt   = "file";
            print( "$acc\t$pt\t$crc\t$pfm\t$time\t$tdb\t$ldr\n" );
        }
        elsif ( -d "$path" )
        {
            my $crc  = get_schema_crc( $acc );
            my $pfm  = get_platform( $acc );
            my $time = get_obj_time( $path );
            my $tdb  = get_tabdb( $acc );
            my $ldr  = get_loader( $path );
            my $pt   = "dir";
            print( "$acc\t$pt\t$crc\t$pfm\t$time\t$tdb\t$ldr\n" );
        }
        else
        {
            print( "$acc not-found\n" );
        }
    }
}

sub get_schema_crc
{
    my( $src ) = @_;
    my $cmd = "vdb-dump $src -A | md5sum";
    open ( PROC, "-|", "$cmd" ) or die "vdb-dump ( print schema ) failed";
    my $row = <PROC>;
    chomp( $row );
    close ( PROC );
    my @parts = split( / /, $row );
    return $parts[ 0 ];
}

sub get_platform
{
    my( $src ) = @_;
    my $cmd = "vdb-dump $src -N -R1 -CPLATFORM 2>/dev/null";
    open ( PROC, "-|", "$cmd" ) or die "vdb-dump ( print platform ) failed";
    my $row = <PROC>;
    chomp( $row );
    close ( PROC );
    return $row;
}

sub get_obj_time
{
    my( $src ) = @_;
    my $cmd = "ls -l -d $src 2>/dev/null";
    open ( PROC, "-|", "$cmd" ) or die "ls -l failed";
    my $row = <PROC>;
    chomp( $row );
    close ( PROC );
    my @parts = split( ' ', $row );
    my $res = sprintf( "%s-%s-%s", $parts[ 5 ], $parts[ 6 ], $parts[ 7 ] );
    return $res;
}

sub get_tabdb
{
    my( $src ) = @_;
    my $cmd = "vdb-dump $src -E 2>/dev/null";
    open ( PROC, "-|", "$cmd" ) or die "vdb-dump ( check tab/db ) failed";
    my $row = <PROC>;
    chomp( $row );
    close ( PROC );
    my @parts = split( / /, $row );
    if ( $row )
    {
        if ( $parts[ 0 ] eq "cannot" )
        {
            return "table";
        }
        else
        {
            return "database";
        }
    }
}

sub get_loader
{
    my( $src ) = @_;
    my $cmd = "kdbmeta $src SOFTWARE/loader 2>/dev/null";
    open ( PROC, "-|", "$cmd" ) or die "kdmeta failed";
    my $row = <PROC>;
    close ( PROC );
    if ( $row )
    {
        chomp( $row );
        return $row;
    }
    else
    {
        return "unknown";
    }
}
