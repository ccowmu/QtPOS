# POS

An attempt at a Point of Sale system for the club office.

## Hacking ##

It is best to use QtCreator as your IDE, or at least use QtDesigner to modify
the .ui file.

We use Qt5.5 and C++11

## Prices ##

Create a file `prices.csv` in the same directory as the `POS` binary. Each
line is an item in the form:

```
item-name,price-in-cents
```

A positive "price" actually adds to the user's wallet (when someone pays club),
and an negative prices subtracts from it (when someone buys something). This
feels backwards and should probably change.

There is currently a bug whereby there can be no spaces in the name or price,
and the file is not a true CSV in that there can be no other commas embedded
in either field.
