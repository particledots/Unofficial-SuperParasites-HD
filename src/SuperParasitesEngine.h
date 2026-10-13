// JUCE-free wrapper around the SuperParasites DSP (MIT licensed: Emilie Gillet's Clouds, Matthias Puech's Parasites,
// Julius Kammerl's Beat Repeat / Spectral Clouds, merged by Patrick Dowling for Supercell/Microcell hardware).
// Unofficial SuperParasites HD: the DSP can run at 32 kHz (the original; 16 kHz in the low-fidelity qualities) in 32-frame
// blocks, or natively at 48 kHz (still 16 kHz in the low-fidelity qualities) in 48-frame blocks, both 1 ms, with 16-bit
// samples; this wrapper converts to and from the host rate (and does not resample at all when the host runs at the engine rate).
#pragma once
#include <memory>
#include <vector>
#include <cstdint>
#include "Resampler.h"

namespace clouds { class GranularProcessor; }

class SuperParasitesEngine
{
public:
    // The module's eight modes, in the order its mode button steps through them.
    enum Mode { Granular = 0, PitchShifterStretch, LoopingDelay, Spectral, Oliverb, Resonestor, BeatRepeat, SpectralClouds, NumModes };

    struct Params
    {
        // Same meaning as the module's knobs (all 0..1 unless noted).
        float position = 0.5f, size = 0.5f, density = 0.7f, texture = 0.5f;
        float pitchPot = 0.5f;                               // the PITCH knob position, 0..1 (the module maps it through a detented curve)
        float pitchOffsetSemitones = 0.0f;                   // added on top, like the V/OCT input (used for MIDI follow)
        // The module's four BLEND roles, each as its own value:
        float dryWet = 0.5f, stereoSpread = 0.5f, feedback = 0.0f, reverb = 0.0f;
        bool freeze = false;
        bool reverse = false;    // granular reverse (the module's reverse toggle)
        bool gate = false;       // the module's gate input; in Spectral madness it glitches the spectrum
        int mode = Granular;
        int quality = 0;     // 0 stereo 16-bit, 1 mono 16-bit, 2 stereo 8-bit low-fi, 3 mono 8-bit low-fi (the module's order)
        int bufferLength = 0; // index into bufferSeconds(): how much sample memory the recording buffer gets (Extended)
        int engineRate = 0;   // 0 = 32 kHz (the original), 1 = 48 kHz (HD)
    };

    // Recording-buffer sizes offered by the Extended build. Index 0 is the module's own memory (about 1 second per channel at
    // 16-bit); the others are longer. The low-fi qualities hold 4x as long (8-bit at 16 kHz), mono holds a little over 2x as long
    // as stereo, exactly as on the module. The numbers are the stereo 16-bit length per channel in seconds.
    static constexpr int kNumBufferLengths = 4;
    static float bufferSeconds (int index);

    SuperParasitesEngine();
    ~SuperParasitesEngine();

    void prepare (double hostSampleRate);

    // Press the module's CAPTURE button / fire its capture input once (seeds a grain, taps tempo, triggers a burst...).
    void capture() { capturePending = true; }

    void process (const float* inL, const float* inR, float* outL, float* outR, int numSamples, const Params& p);

    int latencyInHostSamples() const;
    double processorRate() const { return procRate; }
    static double engineRateHz (int engineRate) { return engineRate == 0 ? 32000.0 : 48000.0; }
    size_t blockSize() const { return kBlock; }

    // The PITCH knob's detented curve: knob position 0..1 -> semitones (-24..+24), as in the module.
    static float pitchPotToSemitones (float pot);

private:
    struct Frame { float l, r; };

    void runBlocks (const Params& p);
    void rebuildProcessor (int bufferLength, int engineRate);   // fresh processor with the chosen sample memory and engine rate
    void setEngineRate (int engineRate);       // switches the rate: new resamplers, empty queues

    std::unique_ptr<clouds::GranularProcessor> proc;
    std::vector<uint64_t> largeBuf, smallBuf;   // the module's 118784-byte main block and 65408-byte CCM block

    double hostRate = 48000.0, procRate = 32000.0;
    size_t kBlock = 32;                         // frames per block: 1 ms (32 at 32 kHz, 48 at 48 kHz)
    int currentEngineRate = 0, lastEngineRateSeen = 0;

    // smoothed copies of the knob values (the firmware low-passes each knob; same coefficients)
    float sPosition = 0.5f, sSize = 0.5f, sDensity = 0.7f, sTexture = 0.5f, sPitchPot = 0.5f, sNote = 0.0f;
    float sDryWet = 0.5f, sSpread = 0.5f, sFeedback = 0.0f, sReverb = 0.0f;
    bool lastFreeze = false, capturePending = false, smoothInit = false;
    int currentBufferLength = -1;

    SincResampler rsInL, rsInR, rsOutL, rsOutR;
    std::vector<float> tmpL, tmpR;
    std::vector<float> hostQL, hostQR;
    size_t hostQHead = 0;
    std::vector<Frame> inQueue;
    size_t inQueueHead = 0;
    std::vector<float> procOutL, procOutR;
};
