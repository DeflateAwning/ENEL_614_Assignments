# ENEL_614_Assignments
 
## Big Notes
* Get to "Project Properties" by searching it in the top-right search bar.
* Model number: `PIC24F16KA102`

## Lessons Learned
1. When working with uint32_t variables and single-bit bitmasks, you must use: `(1UL << bit_pos)` instead of just `(1 << bit_pos)`.
2. In the IDE, you must right-click on the project. Click Properties. Go to "PICkit 4". Select Option Categories > Power. Enable "Power Target Circuit from PICkit 4".
3. In a decreasing loop like this, you must use an `int8_t`, and not a `uint8_t`:

```c
for (int8_t bit_place = 31; bit_place >= 0; bit_place--) {
    ...
}
```
