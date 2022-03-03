> **Disclaimer**: Matlab function is modded a little bit to match the delay time with our implementation (the original one have way too much excessive delay time and delay length). Also the plus in the TAP calculation is changed to minus to flipping the mod phase 180 degrees to match behavior.

For simple sine wave, we expect it to run pretty similar

![alt text](https://github.com/SileYin/2022-MUSI6106/blob/assignment2_vibrato/plots/vibrato_440Hz_comparison.png?raw=true)

For real world example, especially with 44.1kHz (evil), which may have different rounding behavior since 1ms here is not an integer, and bring some errors.

![alt text](https://github.com/SileYin/2022-MUSI6106/blob/assignment2_vibrato/plots/realworld_vibrato_comparison.png?raw=true)
