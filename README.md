# varfs
Super simple, but versatile, 9p filesystem serving multipurpose variables.

`usage: varfs [-n fsname] [-m mtpt] [var ...]`

# Compile
*(assuming plan9port is installed)*

`% mk`
# Example
Start a filesystem named 'fortunes' mounted in /mnt/fortunes, with two variables 'ken' and 'dmr'.
Then add a new variable named rob, and write somr fortunes in it and remove ken and dmr variables.
```
% varfs -n fortunes -m /mnt/fortunes ken dmr
% echo rob >/mnt/fortunes/new
% ls /mnt/fortunes
del
dmr
ken
new
rob
% grep '\- rob' /usr/local/plan9/lib/fortunes >/mnt/fortunes/rob
% p /mnt/fortunes/rob
If it's Weinberger, I'm going with Ditzel. - rob
It's not easy being Joan. -- rob
no more than 1 lp fortune per day -- rob
20 octets is 160 guys playing flutes -- rob
...
% echo ken dmr >/mnt/fortunes/del
% ls /mnt/fortunes
del
new
rob
```
To do the same thing with plan9port `9p(1)`.
```
% varfs -n fortunes ken dmr
% echo rob |9p write fortunes/new
% 9p ls fortunes
del
dmr
ken
new
rob
% grep '\- rob' /usr/local/plan9/lib/fortunes |9p write fortunes/rob
% 9p read fortunes/rob
If it's Weinberger, I'm going with Ditzel. - rob
It's not easy being Joan. -- rob
no more than 1 lp fortune per day -- rob
20 octets is 160 guys playing flutes -- rob
...
% echo ken dmr |9p write fortunes/del
% 9p ls fortunes
del
new
rob
```
