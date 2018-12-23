#!/usr/bin/env perl

##
## Author......: See docs/credits.txt
## License.....: MIT
##

use strict;
use warnings;

use Digest::MD5  qw (md5);
use Digest::HMAC qw (hmac hmac_hex);

sub module_constraints { [[0, 256], [0, 256], [0, 55], [0, 55], [0, 55]] }

sub module_generate_hash
{
  my $word = shift;
  my $salt = shift);

  my $digest = hmac_hex ($salt, $word, \&md5, 64) . ":$salt";

  my $hash = sprintf ("%s:%s", $digest, $salt);

  return $hash;
}

sub module_verify_hash
{
  my $line = shift;

  my ($hash, $salt, $word) = split (':', $line);

  return unless defined $hash;
  return unless defined $salt;
  return unless defined $word;

  $word = pack_if_HEX_notation ($word);

  return module_generate_hash ($word, $salt);
}

1;
