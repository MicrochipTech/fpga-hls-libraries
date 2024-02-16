These examples are used to create the error plots in [../../utils/generators/readme.md](../../utils/generators/readme.md). To generate
an error plot, enter the directory of choice and run `python plot.py`.

This script will switch out DECIM/TABLE_SIZE with a test value, regenerate the table, and then compile & run the example. The error will be 
measured. This will be repeated until we have enough data to generate a graph of error against DECIM/TABLE_SIZE. 
