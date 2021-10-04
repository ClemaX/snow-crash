#!/usr/bin/env perl

use strict;
use warnings;

my ($des_hash, $des_salt) = @ARGV;

if (not defined $des_hash or not defined $des_salt) {
	die "Usage: $0 des_hash des_salt [wordlist]\n";
}

shift(@ARGV); shift(@ARGV);

my $word = "";
my $curr= "";

while (defined($word = <>)) {
	chomp($word);

	$curr = crypt($word, $des_salt);

	if ($curr eq $des_hash) {
		last;
	}
}

if (defined($word)) {
	print "$word\n";
} else {
	die "Word not found\n";
}
