# Unofficial SuperParasites HD — AU / VST3 (experiment)

A copy of [Unofficial SuperParasites Extended](https://github.com/particledots/Unofficial-SuperParasites-Extended) (SuperParasites with the long
**Buffer Length** menu) with one new menu: **Engine Rate**.

* **32 kHz (original)** — the DSP runs at 32 kHz exactly as in Extended (16 kHz in the low-fi qualities). This setting is the default.
* **48 kHz (HD)** — the same DSP running natively at 48 kHz. Delay lengths, window lengths, filter cutoffs and smoothing speeds are rescaled so that
  times in seconds and pitches in Hz match the 32 kHz version, and the spectral modes use a 6144-point FFT instead of 4096. You can switch between the two
  in one plug-in and compare them by ear.

It is a separate plug-in (code `SPHD`, "particledots: Unofficial SuperParasites HD"), so it installs next to Extended and the original.
Not a Mutable Instruments product, and not made or endorsed by any of the module makers.

**I have not listened to it, and nothing here claims the 48 kHz engine sounds the same as, or better than, the 32 kHz one.** The tests below measure times,
pitches, decay times and spectra. Whether the difference is audible, and which you prefer, is for you to say.

## Waveform display with a Position marker (version 0.5.0)
A new panel at the top of the window draws the **recording buffer** (loudest peak per column, oldest on the left, "now" on the right) and a **marker at the Position setting**:

| Mode | Marker | Shaded band |
|---|---|---|
| Granular | where the grains start reading, as time back from now | the stretch one grain plays |
| Pitch shifter / time stretch | how far back the window is read from | the window (it follows Size) |
| Looping delay, running | the delay time (Size is not used) | none |
| Looping delay, Frozen | where the loop begins | the loop (Size is its length) |
| Beat repeat | where the loop starts inside the latched slice | the loop, with the slice outlined; changes take effect at the next slice |
| Spectral madness, Spectral clouds, Resonestor | greyed out: these modes have no recording buffer | |
| Oliverb | greyed out: Position is pre-delay there, not a place in a recording | |

With **Freeze** the picture holds still (except in Beat repeat, which keeps recording while frozen, as the firmware does). The display only reads the sound memory
on the audio thread, a little at a time, and only while a plug-in window is open; the sound is bit-for-bit the same with the window open or closed (tested).

**Verified (Linux, headless; `tests/waveform_test.cpp`, run in the workflow):** where a burst of sound lands in the picture (many formats, rates, buffer lengths);
Freeze; hidden modes; output identical with display open or closed; a reader thread hammering the picture while audio runs; and, with a gliding test tone as a time stamp,
that the sound really comes from the marker (Pitch shifter, Looping delay: within a few ms) or from inside the band (Granular: grains have a lopsided window, so the heard centre sits inside the band).
**Not verified:** how the panel looks and behaves in Logic (it is drawn by code that CI compiles but nobody has seen), and the AU on the Intel Mac.

## Pitch Snap (version 0.4.0)
A new **PITCH SNAP** menu (parameter `pitch_snap`, to the right of the mode hint) settles the PITCH knob on musical notes, as in the Unofficial Spotykach and Unofficial Beads plug-ins:
* **Off (the default)** is exactly the 0.3.0 sound: the module's own detented knob curve.
* **Steps** snaps to octaves and fifths: -24, -12, -7, 0, +7, +12, +24 semitones.
* **Semitones** snaps to whole semitones.
With Steps or Semitones the PITCH readout shows the note that actually plays (a whole number such as -12 or +7), not the raw knob position. The snap is applied to the knob's value before the MIDI note
(Pitch Follows MIDI) is added, and the change glides over a few milliseconds so a step is not a click. It applies to the modes that shift pitch in semitones (Granular, Pitch shifter / time stretch, Looping delay,
Oliverb, Resonestor). In Beat repeat and Spectral clouds the firmware reads the knob position itself rather than a pitch, so the snap does not apply there. Projects saved before 0.4.0 have no such parameter and load with Pitch Snap **Off**.
There is no scale menu here: unlike Beads, this engine has no random pitch for a scale to snap.
**Verified (Linux, headless; `tests/pitch_snap_test.cpp`, run in the workflow):** the snapping function is exact over the whole range; across every knob position Steps lands only on those seven values and never moves against the knob;
in the Pitch shifter, a sine's output pitch follows the snapped value (measured within 0.1 semitone of the expected note in 7 knob/snap combinations); every mode stays finite and bounded with each setting and a sweeping knob, at both engine rates.
**Not verified:** how it looks and feels in Logic (the menu position, the readout), and by ear in each mode.

## Limiter switch (version 0.3.0)
A new **LIMITER** button (parameter `limiter`) sits at the end of the button row. **On (the default) is exactly the 0.2.0 sound**: the soft limiter after Output Level keeps loud sounds from going past 0 dBFS.
**Off** removes it: the output is then the engine plus the Output Level gain and nothing else, so loud wet sounds can go past 0 dBFS (by tens of dB at high Output Level settings). Only two safety things
remain: NaN becomes silence and values are held inside +/-16 so a runaway can never reach the host as infinity. Use Output Level, or a limiter after this plug-in, to keep it in range.
Switching glides over about 10 ms, so there is no click. Presets and projects saved before 0.3.0 have no switch and load with the limiter **on**, i.e. they sound as they did in 0.2.0.
Why it exists: the limiter is an instant curve (no attack, no release, no look-ahead), so on very loud sounds it acts like gentle distortion, which can be heard; the switch lets you turn that off and manage the level yourself.

## Output Level (version 0.2.0)
**What was wrong.** Loading any plug-in of this family made the sound quieter than before it was loaded. With Dry/Wet at 0 (nothing but the input) I measured the output at
**-9.04 dB** relative to the input in the six modes that have a dry path (Granular, Pitch shifter / time stretch, Looping delay, Spectral madness, Oliverb, Spectral clouds), at every
host rate tested (44.1, 48, 96 kHz), both engine rates and all four qualities. The cause is in the firmware code, not the wrapper: its dry/wet crossfade table starts at 0.707 (-3 dB) and its
output conversion halves the signal (x 0.5, -6 dB) before the 16-bit output. The module's analogue output stage presumably makes that up; a plug-in has none.

**The fix.** A new **OUTPUT LEVEL** knob (parameter `output_level`, -24 to +24 dB, default **+9.03 dB** = 20 x log10 (2 x sqrt 2)) sits after the engine, followed by a **soft limiter**.
With the default, a fully dry signal comes out at the level it went in. The limiter passes everything below -0.9 dBFS unchanged and bends louder peaks towards 0 dBFS, which they
never exceed; it is an instant curve (no delay, no look-ahead), a safety net against clipping rather than a mastering limiter. The gain glides (about 10 ms) so moving or automating the knob does not click.
The engine and firmware code are untouched, so the 32 kHz setting still equals Extended bit for bit (the workflow checks that). The stage lives in `src/OutputStage.h` and is called from `processBlock`.

**What it does not do.**
* **Beat repeat and Resonestor have no dry path in the firmware**, so "fully dry equals the input" does not apply to them. Beat repeat forces its blend to wet and the input always goes through
  the slice player (the firmware applies a 1.2 x, +1.6 dB, gain to it that the other modes' dry signal does not get); Resonestor's Dry/Wet knob is a distortion amount. Measured at the default Output Level, input -18 dBFS:
  Beat repeat **+1.57 dB** (16-bit), Resonestor **-1.61 dB** (16-bit); in the low-fi qualities Beat repeat is -0.2 dB (stereo 8-bit) and Resonestor -2.6 dB. I left those as they are: whether they should be
  trimmed to match is your call.
* **Beat repeat in Mono 8-bit (low-fi) gives no output at all** in this test (-330 dB, at every Dry/Wet setting). It is in the firmware code path and was there before this change; I have not looked into it.
* Loud dry signals are bent slightly by the firmware's own soft limit, before this stage: Looping delay, dry, input -6 dBFS: -0.06 dB; -1.9 dBFS: -0.16 dB; -0.4 dBFS: -0.22 dB.
* The firmware's output is 16-bit and is 9 dB lower than the input before this gain is applied, so the gain also raises the firmware's own 16-bit rounding noise by 9 dB (it does not add any). I have not measured or listened for it in quiet passages.
* **Presets and projects saved before 0.2.0 have no Output Level**, so they load at the +9.03 dB default and come out about 9 dB louder than they did. That is the fix, but if you
  had turned the channel up to compensate, turn it back down.

## How the two settings relate

| | 32 kHz (original) | 48 kHz (HD) |
|---|---|---|
| Processing rate / block | 32 kHz, 32 frames (1 ms) | 48 kHz, 48 frames (1 ms) |
| Host at 48 kHz | resampled 48 -> 32 -> 48 kHz (sinc) | **no resampling at all** (straight copy) |
| Highest frequency passed | about 15.5 kHz | the host's Nyquist (24 kHz at 48 kHz) |
| Delay times, reverb decay, pitch in Hz, grain/window lengths in ms | the module's | the same (rescaled, see `docs/RATE_AUDIT.md`) |
| Buffer Length (1 / 4 / 10 / 20 s) | seconds as in Extended | the same seconds (1.5x the bytes) |
| Spectral modes (Spectral madness, Spectral clouds) | FFT 4096, window 128 ms, bins 7.8125 Hz apart | FFT **6144**, window 128 ms, bins 7.8125 Hz apart |
| Low-fi qualities (8-bit) | processing at 16 kHz (2:1 filter) | processing at **16 kHz** (new 3:1 filter); same seconds of memory |
| Reverb / Oliverb / Resonestor delay lines | 16384 samples | 32768 samples (longer delay lines in samples, same seconds) |
| Output if the engine rate is 32 kHz | bit-for-bit the same samples as Extended (checked, see below) | — |

Switching the Engine Rate rebuilds the processor and clears the recorded audio (like changing Buffer Length or Quality), and the plug-in's reported
latency changes slightly.

### What does *not* come out the same (expected, not bugs)

These are consequences of running at a different sample rate, not of the rescaling:

1. **Interpolation brightness.** Reads at fractional positions (Hermite in Beat repeat and the Oliverb loop, linear in grains, pitch shifting) roll off
   at a fixed fraction of the sample rate, so at 48 kHz the roll-off reaches higher in Hz. Measured with noise in: the top octave (6.4 - 12.8 kHz) is up
   to about 4 dB brighter in Oliverb and 5.6 dB brighter in Beat repeat played at half speed; below 6.4 kHz every octave band agrees within the 3 dB the test allows. (Beat
   repeat at normal speed, which has no interpolation, differs by 0.0 dB.)
2. **Content above 16 kHz** exists only in the 48 kHz setting.
3. **Filter shapes near the top of their range.** The module's `FREQUENCY_FAST` filters use a polynomial that is inaccurate at high cutoffs; the same Hz
   is a lower fraction of 48 kHz, so the filter is slightly more accurate there and differs a little from the 32 kHz one. I did not change the polynomial,
   because that would change the 32 kHz output.
4. **Spectral modes**: the window is the original 4096 window re-sampled to 6144 points (the original's analytic form is not known) and re-normalised so that
   overlapping windows still add up exactly. The "warp" curves are applied over the first 0 - 16 kHz of the spectrum (the same Hz as before); bins above 16 kHz
   pass through unwarped.
5. **Delay lengths are whole samples.** A length of n samples at 32 kHz becomes round(1.5 n) at 48 kHz: the time is right to within 10 microseconds.
6. **Random and chaotic parts** (granular jitter, Spectral clouds' random band gains, feedback loops) cannot be compared sample by sample between rates; only
   their statistics can.
7. **CPU.** Roughly 1.0 - 1.7x the cost of the 32 kHz setting in my Linux timing of the headless test (Spectral madness the most). Not measured on a Mac.

### Source changes compared with Extended
Details in `docs/RATE_AUDIT.md`. In short: a small header (`engine_rate.h`) holds the rescaling rules; the delay-line effects (Reverb, Diffuser, Oliverb,
Resonestor, PitchShifter) became templates so their delay lengths are compile-time constants at either rate; players, buffers, filters and smoothing take the
rate at run time; `HdFft` (a 3 x 2^k real FFT) and a 3:1 low-fi converter were added; and the wrapper chooses block size, memory and resamplers per rate.
One change affects **both** rates: `grain.h` and `window.h` keep their 16.16 fixed-point position in a 64-bit integer instead of a 32-bit one (the same kind
of overflow fix Extended made in `looping_sample_player.h` and `kammerl_player.h`). It only changes the output when the old 32-bit value would have overflowed;
in the 64 combinations checked below the 32 kHz output is identical.

## Install
Download `SuperParasites-HD-macOS` from the latest passing run in the Actions tab and follow `INSTALL.txt`
(universal arm64 + x86_64, macOS 11 or later). Controls are the same as in the other plug-ins, plus the Engine Rate menu next to Buffer Length.

## Verified and not verified

**Limiter switch (version 0.3.0): built on GitHub Actions (macOS runner), every step passed, run 39075607770, commit `0140205`** (all earlier tests, the extended level tests and both mutation checks, the universal arm64 + x86_64 AU and VST3 build, signing, packaging). I can see each step's pass / fail, not its log text. Not yet loaded in Logic and not listened to.

**Limiter switch (version 0.3.0) - measured and tested here (Linux, g++, October 2026), not listened to**
* `tests/level_test.cpp`: 10581 checks, 0 failed (the 0.2.0 checks plus the switch): with the limiter on, a loud signal never passes 1.0; off, the same signal comes out at exactly the Output Level gain (within 0.0001) and
  its peak is past 1.5; a quiet signal is identical on and off; NaN and infinity never get through when off; switching glides (no step above 2 % of the whole move) and settles exactly, and switching back on gives exactly the 0.2.0 value.
* `tests/proc_level_test.cpp`: 31 checks, 0 failed, on the real processor class: the parameter exists and is on by default; a preset saved without it loads with the limiter on; with Output Level at +24 dB and a loud wet input
  (four modes) the peak is 1.0000 with the limiter on and well past 1.0 with it off, so the switch really reaches the sound.
* Mutation checks: `tests/mutation_check.sh` now breaks the stage in 7 ways (the earlier 4 plus switch ignored one way and the other, and the off-guard removed) and `tests/wiring_mutation_check.sh` in 6 ways (the earlier 3 plus
  switch not read one way and the other, and default wrong); every broken copy was caught.
* Not tested: how the switch sounds or looks in Logic, whether the button fits the panel as intended (the row is 606 of 612 pixels wide), Logic's handling of the new parameter in an existing project (the plug-in code is unchanged).

**Output Level (version 0.2.0): every step passed on the macOS runner, run 39054152355, commit `56e34fe`** (all earlier tests, the bit-for-bit check of the 32 kHz output against Extended, the four new level steps, the universal arm64 + x86_64 AU and VST3 build, signing, packaging). I can see each step's pass / fail, not its log text. Artifact `SuperParasites-HD-macOS`.

**Output Level (version 0.2.0) - measured and tested here (Linux, g++, October 2026), not listened to**
* `tests/level_test.cpp`: 10,573 checks, 0 failed. Fully dry (Dry/Wet = 0) at the default Output Level, engine + stage output equals the input level within 0.1 dB in each of the six dry-path modes, at 44.1 / 48 / 96 kHz
  host, every engine rate and quality the plug-in has, input -18 dBFS (and -6 dBFS at 48 kHz); worst deviation 0.064 dB. The limiter: identity up to its threshold, odd-symmetric, monotonic, slope 1 at the threshold,
  never above 1.0 for any input including infinity, NaN becomes 0. The gain is exact to 0.001 dB at -24, -6, 0, +9.03 and +12 dB; a step from +9.03 to -24 dB glides with no jump. No clipping: with a 0.95-amplitude input, all modes
  and qualities, Dry/Wet 0 / 0.5 / 1, Output Level at the default and at +24 dB, no sample exceeds 1.0; without the limiter the engine's wet peaks at the default gain would reach about 2.3 x full scale.
* `tests/proc_level_test.cpp`: 24 checks, 0 failed, on the real plug-in processor class (the same `PluginProcessor.cpp` as the AU / VST3, compiled against JUCE 8.0.6, run headless): the parameter exists and its default shows as "+9.03 dB";
  the six dry-path modes come out within 0.013 dB of the input at the defaults; setting Output Level to -24 / -9.03 / 0 / +6 dB moves the output by exactly that much (within 0.02 dB); loud input at +24 dB in every mode stays at or below 1.0;
  a preset saved without an Output Level loads at +9.03 dB (and keeps its other settings), also into an instance whose knob had been turned to 0 dB.
* **Mutation checks (the tests fail when the level is broken).** `tests/mutation_check.sh` breaks `OutputStage.h` in four ways (gain removed, default 8.0 dB instead of 9.03, wrong dB-to-gain factor, limiter removed) and the level test failed for every one
  (17, 13, 19 and 4,035 failed checks), while passing on the unbroken code. `tests/wiring_mutation_check.sh` breaks `processBlock` in three ways (stage not called, level parameter not read, parameter default 0 dB) and the whole-plug-in test failed each
  time (9, 9 and 8 failed checks); the source is restored afterwards. Both are workflow steps.
* The existing suites still pass with the new CMake targets (`engine_test`, `rate_compare_test`); the engine and firmware code are not touched, so the bit-for-bit check against Extended in the workflow is unaffected.
* The editor source (the fifth knob in the second row) compiles against JUCE 8.0.6 with the rest of the plug-in. **How it looks is not checked: I have not seen the editor.**
* **Not verified:** how it sounds; that the level you read on Logic's meters / faders now matches; the 16-bit noise floor at +9 dB in quiet passages; the knob row in Logic; behaviour when automated; any
  match with the real module's output level; Beat repeat and Resonestor levels against a real module.

**Verified here (Linux, g++, October 2026)**
* *32 kHz setting equals Extended.* `tests/bit_exact_dump.cpp` hashes the output of every mode x quality (4) x Buffer Length (1 s, 4 s), 64 combinations, 6 s each,
  with a fixed input and moving knobs. The hashes from this code at 32 kHz and from Extended at commit `66d2161` are identical. The GitHub workflow repeats this
  with the macOS compiler.
* *The 6144-point FFT* matches a plain DFT (error about 1e-7 relative) at 12, 48, 384 and 6144 points and round-trips; the 6144 window is power-complementary
  to 1e-7 and within 0.0003 of the original window's shape.
* *Rescaling rules*: every delay length at 48 kHz is within half a sample of the 32 kHz time; one-pole coefficients give the same decay per second; cutoffs the same Hz.
* *The old test suite* (`engine_test`: all modes x qualities, freeze, mode switching, extreme knobs, Buffer Length 1 / 4 / 10 / 20 s including mono and low-fi,
  44.1 / 48 / 96 kHz hosts) now runs at both engine rates: 254 checks, all pass. (One bound for extreme knob settings was loosened from 1.5 to 1.7 because the
  host-rate resampler overshoots a hard-clipped signal more with a 96 kHz host and the 48 kHz engine; the engine itself never exceeds 1.0.)
* *`rate_compare_test`* (same input and knobs at both rates, measured, 32 kHz vs 48 kHz):
  * delay times: looping delay at nine positions (16-bit and low-fi) agrees within 1.5 ms, which is the resampling latency of the 32 kHz path; granular and
    time-stretch start/peak times agree within 5 ms; Beat repeat's loop period is 0.2500 s at both rates;
  * pitch (+12, -5, +7, -7 semitones etc.): granular, stretch, looping-delay pitch shifter, spectral, spectral clouds, Oliverb and Resonestor agree between the
    rates (limit 0.5 %; 1.2 % for granular and 1.5 % for Oliverb because their output is random or smeared; the largest difference seen was 0.9 %). In some modes **both** rates are off the plain-arithmetic pitch (for example
    the looping-delay pitch shifter gives 702 Hz for +7 semitones from 440 Hz at both rates): that is the original's behaviour, not a rate effect;
  * reverb decay (60 dB time): the post reverb at three settings agrees within 5.4 %, Oliverb at six settings within 3 % (averaged over three noise bursts,
    whose own spread is about 2 - 4 %);
  * spectrum with white noise in, all eight modes, hi-fi and low-fi: each octave band 100 Hz - 12.8 kHz agrees in shape within 3 dB (6 dB in the top octave, see item 1
    above); total level within 2 dB.
* *GitHub Actions on macOS (run 38476158548, commit `69a40f7`, passed):* the universal arm64 + x86_64 AU and VST3 build, the ad-hoc signing and packaging, `engine_test` at
  both engine rates, `rate_compare_test`, and the bit-for-bit comparison of the 32 kHz output with Extended built with the macOS compiler. (I can only see that the steps
  exited successfully, not their log: the log host is not reachable from where I work.) The first run failed because AppleClang wants the `template` keyword in
  `resonestor.h`; fixed.

**Not verified**
* Loading in Logic on the M4 Mac mini or the Intel Mac; whether the Engine Rate switch behaves in a host (it rebuilds the processor on the audio thread, like Buffer Length).
* **How it sounds**, and whether 48 kHz sounds the same as, better than or worse than 32 kHz in any mode. I have not listened. Likewise no comparison with a hardware module.
* CPU use on a Mac.
* Any host sample rate other than 44.1 / 48 / 96 kHz; 48 kHz engine with a 44.1 kHz host uses the same sinc resampler as the 32 kHz one.
* The accuracy of the replaced low-fi converter (69 taps, 3:1) beyond the spectrum test above.

**Found in the original code and left unchanged** (changing them would break the bit-exact match with Extended)
* `correlator.cc`: `x >> (32 - offset_bits)` is an undefined shift when `offset_bits` is 0. On the module's CPU this gives 0; on Intel and Apple CPUs it gives `x`, so 1 in 32
  candidate positions is scored wrongly in Pitch shifter / time stretch and Oliverb's pre-delay.
* A few 4-byte reads just past the end of small tables (`slope_response`, `lut_xfade_in/out`) when a knob is at an end stop.

## Planned experiments
1. Done: long buffers at 32 kHz (Extended).
2. Done here, unlistened: native 48 kHz. Possible follow-ups depending on what you hear: 44.1 kHz, or 96 kHz.
3. Later, maybe: several captured layers (the Instruō Arbhar idea).

## Source and licence
Same as Extended: `third_party/supercell/` is `supercell/dsp/*` and `supercell/resources.*` from `patrickdowling/superparasites` (commit `2e9ed7c`) with the HD
changes above, `third_party/stmlib/` is a subset of `mqtthiqs/stmlib` at `8ab2aaee77cbacb47b646d46d22ee5d358effe2d`. MIT per that project's README, with MIT notices
in the source files (Emilie Gillet, Matthias Puech, Julius Kammerl, Patrick Dowling); keep them if you redistribute. The wrapper code in `src/` has no licence assigned
yet. JUCE is used under its own terms (AGPLv3 / commercial).
