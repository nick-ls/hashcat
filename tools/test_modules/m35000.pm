#!/usr/bin/env perl

##
## Author......: See docs/credits.txt
## License.....: MIT
##

use strict;
use warnings;

use Crypt::Digest::BLAKE2s_256 qw (blake2s_256_hex);

sub module_constraints { [[0, 128], [-1, -1], [0, 64], [-1, -1], [-1, -1]] }

sub module_generate_hash
{
  my $word = shift;

  my $digest = blake2s_256_hex ($word);

  my $hash = sprintf ("\$BLAKE2\$" . lc ($digest));

  return $hash;
}

sub module_verify_hash
{
  my $line = shift;

  my ($hash, $word) = split (':', $line);

  return unless defined $hash;
  return unless defined $word;

  my $word_packed = pack_if_HEX_notation ($word);

  my $new_hash = module_generate_hash ($word_packed);

  return ($new_hash, $word);
}

1;
