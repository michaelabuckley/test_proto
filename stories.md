# Stories

## Current
- new library - https://github.com/Molorius/ATSAMD21-ADC

## Next 
- baseline testcase - manual short of all probes.  Sample with 10R. 


## Done
- refactor standard to raw calls - compare - OK
- try differential, but buggy on C-C1.

### Self calibrate with an extra pin beside sample pin.

Wire a 10R resitor to A3, and sample with TA_Drv high.  Alternate TA read with TA
//#define PIN_TA_10 PIN_A3
