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
use LWP::Simple;

use Data::Dumper;

my %opts = (
    host   => 'www.ncbi.nlm.nih.gov',
    cgi    => 'projects/variation/tools/1000genomes/genomesearch.cgi',
    assm   => 'GCF_000001405.17',
    filter => ''
);

sub usage()
{
    print <<"HELP";
convert gene names to chromosome and range

Usage:
$0 <name>...
options are
    -h | -? | --help    this message

Example:
    $0 williams

HELP
    exit 0;
}

#@ARGV = ( 'cat' ) if scalar @ARGV == 0;
usage if scalar @ARGV == 0;
foreach (@ARGV) {
    usage() if (/^-h$/ || /^-\?$/ || /^--help$/);
}

package JSON;

sub _json_object;
sub _json_array;
sub _json_value;
sub _json_value_check;

use constant {
    cc_WHITESPACE   =>  1,
    cc_OPERATOR     =>  2,
    cc_QUOTE        =>  4,
    cc_ESCAPE       =>  8,
    cc_ESCAPED      => 16,
    cc_UNICODED     => 32
};

my @CC = map {
    my $x = 0;
    
    $x |= cc_WHITESPACE if (chr($_) =~ /\s/);
    $x |= cc_OPERATOR   if (chr($_) =~ /[\[\]{},:]/);
    $x |= cc_QUOTE      if (chr($_) eq '"');
    $x |= cc_ESCAPE     if (chr($_) eq '\\');
    $x |= cc_ESCAPED    if (chr($_) =~ /[\\\/bfnrt]/);
    $x |= cc_UNICODED   if (chr($_) eq 'u');
    $x;
} (0..255);

sub _json_token_string
{
    my ($sref, $i) = @_;
    my $slen = length($$sref);
    my $st = 1;
    my $string = '';
    
    for ( ; $$i < $slen; ++$$i) {
        local $_ = substr($$sref, $$i, 1);
        my $cc = $CC[ord($_)];
        
        if ($st == 1) { # string tokenizing
            if ($cc & cc_QUOTE) {
                ++$$i;
                return ('string', eval "\"$string\"")
            }
            if ($cc & cc_ESCAPE) {
                ++$st;
            }
            else {
                $string .= $_;
            }
            next;
        }
        if ($st == 2) { # escape tokenizing
            if ($cc & cc_ESCAPED) {
                $string .= "\\$_";
            }
            elsif ($cc & cc_UNICODED) {
                my $value = substr($$sref, $$i + 1, 4);

                return (undef, 'expected 4 hex digits') unless $value =~ /[[:xdigit:]]{4}/;
                $string .= "\\N{U+$value}";
                $$i += 4;
            }
            else {
                $string .= "\\\\$_";
            }
            $st = 1;
            next;
        }
    }
}

sub _json_token
{
    my ($sref, $i) = @_;
    my $slen = length($$sref);
    
    for ( ; $$i < $slen; ++$$i) {
        local $_ = substr($$sref, $$i, 1);
        my $cc = $CC[ord($_)];
        
        next if ($cc & cc_WHITESPACE);

        if ($cc & cc_QUOTE) {
            ++$$i;
            return _json_token_string(@_);
        }
        if ($cc & cc_OPERATOR) {
            ++$$i;
            return ('op', $_);
        }
        if (substr($$sref, $$i, 5) eq 'false') {
            $$i += 5;
            return ('bool', 0);
        }
        if (substr($$sref, $$i, 4) eq 'true') {
            $$i += 4;
            return ('bool', 1);
        }
        if (substr($$sref, $$i, 4) eq 'null') {
            $$i += 4;
            return ('null', undef);
        }
        if (substr($$sref, $$i) =~ /^(-{0,1}\d+(?:\.\d+){0,1}(?:e[-+]{0,1}\d+){0,1})/i) {
            my $value = $1;
            
            $$i += length $value;
            return ('number', eval $value);
        }
        substr($$sref, $$i) =~ /(\w+)/;
        return (undef, "unexpected '$1'");
    }
    return ('empty', 'no input');
}

sub _json_value
{
    my $start = ${$_[1]};
    my ($type, $value) = _json_token(@_);
    my $len = ${$_[1]} - $start;
    
    $len = 10 if $len < 10;
    die substr(${$_[0]}, $start, $len)."\nJSON syntax error: $value" unless $type;
    if ($type eq 'op') {
        if ($value eq '{') {
            $type = 'object';
            $value = _json_object(@_);
        }
        elsif ($value eq '[') {
            $type = 'array';
            $value = _json_array(@_);
        }
    }
    return ($type, $value);
}

sub _json_value_check
{
    my ($type, $value) = @_;
    
    return $value if (   $type eq 'array'
                      || $type eq 'bool'
                      || $type eq 'null'
                      || $type eq 'number'
                      || $type eq 'object'
                      || $type eq 'string');

    die "JSON syntax error: expected array, bool, null, object, or string";
}

sub _json_object
{
    my $rslt;

    for ( ; ; ) {
        my ($type, $value) = _json_token(@_);
        my $name;

        unless (defined($rslt)) {
            $rslt = {};
            last if ($type eq 'op' && $value eq '}');
        }
        die "JSON syntax error: expected string" unless $type eq 'string';
        $name = $value;

        ($type, $value) = _json_token(@_);
        die "JSON syntax error: expected ':', have '$value'" unless ($type eq 'op' && $value eq ':');

        $rslt->{$name} = _json_value_check(_json_value(@_));

        ($type, $value) = _json_token(@_);
        die "JSON syntax error: $value" unless defined($type);
        last if ($type eq 'op' && $value eq '}');
        die "JSON syntax error: expected ',' or '}', have '$value'" unless ($type eq 'op' && $value eq ',');
    }
    return $rslt;
}

sub _json_array
{
    my $rslt;

    for ( ; ; ) {
        my ($type, $value) = _json_value(@_);
    
        unless (defined($rslt)) {
            $rslt = [];
            last if ($type eq 'op' && $value eq ']');
        }
        push @{$rslt}, _json_value_check($type, $value);

        ($type, $value) = _json_token(@_);
        last if ($type eq 'op' && $value eq ']');
        die "JSON syntax error: expected ',' or ']', have '$value'" unless ($type eq 'op' && $value eq ',');
    }
    return $rslt;
}

sub decode()
{
    my $self = $_[0];
    my $i = 0; # holds current parsing position
    
    return _json_value_check(_json_value(\$self->{'source'}, \$i));
}

sub new($)
{
    my $class = $_[0];
    my $self = { 'source' => ($_[1] || '') };
    
    bless $self, $class;
    return $self;
}

package main;

sub get_object_locations($)
{
    my $response = get("http://$opts{'host'}/$opts{'cgi'}?assm=$opts{'assm'}&query=$_[0]");
    my $json     = new JSON($response);
    my $parsed   = $json->decode();
    
    return $parsed->{'results'}->{'obj_locs'};
}

sub process($)
{
    my $obj_locs = get_object_locations($_[0]);

    foreach ( @{$obj_locs} ) {
        my $type = $_->{'feature_type'};
        
        if (!$opts{'filter'} || $type eq $opts{'filter'}) {
            my $label = $_->{'label'};
            my $chrom = $_->{'chr'}->{'chrom'};
            my $from = $_->{'seq_from'};
            my $to = $_->{'seq_to'};
        
            printf("%s: %s\tslice: '%s:%u-%u'\n",
                   $type, $label, $chrom, $from, $to);
        }
    }
}

for (my $i = 0; $i < scalar @ARGV; ++$i) {
    $_ = $ARGV[$i];
    
    if (/^-/) {
        if (/^--filter/) {
            $opts{'filter'} = $ARGV[++$i];
            next;
        }
        if (/^--assembly/) {
            $opts{'assm'} = $ARGV[++$i];
            next;
        }
        usage();
    }
    process $_;
}
